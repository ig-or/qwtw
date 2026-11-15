"""Python ctypes wrapper for libqwtw (out-of-process Qt6/Qwt plotting library).

Supports both NumPy arrays (fast contiguous buffer path) and plain Python
lists/tuples (zero-dependency fallback via the standard library `array` module).
"""

from __future__ import annotations

from array import array as _c_array
import ctypes
from ctypes import (
    CDLL,
    CFUNCTYPE,
    POINTER,
    c_char_p,
    c_double,
    c_int,
    c_uint,
    create_string_buffer,
)
import math
import os
from pathlib import Path
import platform

try:
    import numpy as np  # type: ignore
except ImportError:
    np = None  # type: ignore

qq: CDLL | None = None
_active_picker_cb = None
_active_clip_cb = None

# Callback C function types matching qwtw/proclib/qwtw.h
_PICKER_CB_TYPE = CFUNCTYPE(
    None,
    c_int,     # figureID
    c_int,     # lineID
    c_int,     # index
    c_int,     # fx
    c_int,     # fy
    c_double,  # x
    c_double,  # y
    c_double,  # z
    c_double,  # t
    c_char_p,  # legend
)
_CLIP_CB_TYPE = CFUNCTYPE(None, c_double, c_double, c_int)


def _find_library(explicit_path: str | os.PathLike | None = None) -> str:
    sys_name = platform.system()
    machine = platform.machine().lower()

    if sys_name == "Darwin":
        lib_names = ["libqwtw.dylib", "libqwtwd.dylib"]
        plat_dirs = [f"macos-{machine}", "macos-arm64", "macos-x86_64"]
    elif sys_name == "Linux":
        lib_names = ["libqwtw.so", "libqwtwd.so"]
        plat_dirs = [f"linux-{machine}", "linux-aarch64", "linux-x86_64"]
    else:
        lib_names = ["libqwtw.dll", "qwtw.dll", "libqwtwd.dll", "qwtwd.dll"]
        plat_dirs = ["win32_build"]

    for raw in (explicit_path, os.environ.get("QWTW_LIB_PATH")):
        if not raw:
            continue
        p = Path(raw)
        if p.is_file():
            return str(p)
        if p.is_dir():
            for name in lib_names:
                cand = p / name
                if cand.is_file():
                    return str(cand)
        if explicit_path and raw is explicit_path:
            return str(explicit_path)

    repo_root = Path(__file__).resolve().parent.parent
    search_dirs: list[Path] = [Path.cwd()]
    for pd in plat_dirs:
        search_dirs.append(repo_root / "lib" / pd / "release")
        search_dirs.append(repo_root / "lib" / pd / "debug")

    for d in search_dirs:
        for name in lib_names:
            candidate = d / name
            if candidate.exists():
                return str(candidate)

    return lib_names[0]


def _bind_signatures(lib: CDLL) -> None:
    lib.get42.argtypes = [c_int]
    lib.get42.restype = c_int

    if hasattr(lib, "qtstart"):
        lib.qtstart.restype = c_int
    if hasattr(lib, "qtstart_debug"):
        lib.qtstart_debug.restype = c_int

    lib.qwtclose.argtypes = []
    lib.qwtclose.restype = None

    lib.qwtversion.argtypes = [c_char_p, c_int]
    lib.qwtversion.restype = c_int

    lib.qwtfigure.argtypes = [c_int, c_uint]
    lib.qwtfigure.restype = c_int

    lib.qwtspectrogram.argtypes = [c_int, c_uint]
    lib.qwtspectrogram.restype = c_int

    lib.spectrogram_info.argtypes = [
        c_int, c_int,
        c_double, c_double, c_double, c_double,
        POINTER(c_double),
    ]
    lib.spectrogram_info.restype = c_int

    lib.spectrogram_info2.argtypes = [
        c_int, c_int,
        c_double, c_double, c_double, c_double,
        POINTER(c_double), POINTER(c_double), POINTER(c_double),
    ]
    lib.spectrogram_info2.restype = c_int

    lib.qwtclipgroup.argtypes = [c_int]
    lib.qwtclipgroup.restype = None

    lib.qwttitle.argtypes = [c_char_p]
    lib.qwttitle.restype = None

    lib.qwtxlabel.argtypes = [c_char_p]
    lib.qwtxlabel.restype = None

    lib.qwtylabel.argtypes = [c_char_p]
    lib.qwtylabel.restype = None

    lib.qwtclear.argtypes = []
    lib.qwtclear.restype = None

    lib.qwtshowmw.argtypes = []
    lib.qwtshowmw.restype = None

    lib.qwtsetimpstatus.argtypes = [c_int]
    lib.qwtsetimpstatus.restype = None

    lib.qwtplot.argtypes = [
        POINTER(c_double), POINTER(c_double),
        c_int, c_char_p, c_char_p, c_int, c_int,
    ]
    lib.qwtplot.restype = c_int

    lib.qwtplot2.argtypes = [
        POINTER(c_double), POINTER(c_double),
        c_int, c_char_p, c_char_p, c_int, c_int,
        POINTER(c_double),
    ]
    lib.qwtplot2.restype = c_int

    lib.qwtremove.argtypes = [c_int]
    lib.qwtremove.restype = None

    lib.qwtchange.argtypes = [
        c_int,
        POINTER(c_double), POINTER(c_double),
        POINTER(c_double), POINTER(c_double),
        c_int,
    ]
    lib.qwtchange.restype = c_int

    lib.qwtsave_png.argtypes = [c_int, c_char_p]
    lib.qwtsave_png.restype = c_int

    lib.qwtsetpos.argtypes = [
        c_int,
        POINTER(c_int), POINTER(c_int),
        POINTER(c_int), POINTER(c_int),
        c_int,
    ]
    lib.qwtsetpos.restype = c_int

    if hasattr(lib, "setcallback"):
        lib.setcallback.argtypes = [_PICKER_CB_TYPE]
        lib.setcallback.restype = None
    if hasattr(lib, "setclipcallback"):
        lib.setclipcallback.argtypes = [_CLIP_CB_TYPE]
        lib.setclipcallback.restype = None
    if hasattr(lib, "qwtmap"):
        lib.qwtmap.argtypes = [c_int]
        lib.qwtmap.restype = c_int


def _ensure_started() -> CDLL:
    if qq is None:
        raise RuntimeError("qwtwc.start() has not been called")
    return qq


def _as_c_double_array(arr) -> tuple[object, int, POINTER(c_double)]:
    """Return (owner_ref, length, ctypes_double_ptr) for a 1D/flattened sequence."""
    if np is not None and isinstance(arr, np.ndarray):
        contig = np.ascontiguousarray(arr, dtype=np.float64).ravel()
        return contig, int(contig.size), contig.ctypes.data_as(POINTER(c_double))
    if np is not None:
        contig = np.ascontiguousarray(arr, dtype=np.float64).ravel()
        return contig, int(contig.size), contig.ctypes.data_as(POINTER(c_double))

    buf = _c_array("d", arr)
    n = len(buf)
    c_buf = (c_double * n).from_buffer(buf)
    return (buf, c_buf), n, ctypes.cast(c_buf, POINTER(c_double))


def _to_2d_rows(x) -> list[list[float]]:
    if np is not None:
        arr = np.asarray(x, dtype=np.float64)
        if arr.ndim != 2:
            raise ValueError("Expected a 2D matrix")
        return arr.tolist()
    rows = [list(r) for r in x]
    if not rows or not isinstance(rows[0], list):
        raise ValueError("Expected a 2D matrix (list of lists)")
    return rows


def start(lib_path: str | os.PathLike | None = None, debug: int = 0) -> int:
    """Load libqwtw and connect to (or launch) the qwproc plot server."""
    global qq
    resolved = _find_library(lib_path)
    qq = CDLL(resolved)
    _bind_signatures(qq)

    assert qq.get42(0) == 42
    if debug > 0:
        qq.qtstart_debug.argtypes = [c_int]
        rc = qq.qtstart_debug(debug)
    else:
        qq.qtstart.argtypes = []
        rc = qq.qtstart()
    if rc != 0:
        raise RuntimeError(f"qtstart failed with error code {rc}")
    return rc


def close() -> None:
    """Shut down the qwproc server process."""
    if qq is not None:
        qq.qwtclose()


def version() -> str:
    lib = _ensure_started()
    vs = create_string_buffer(450)
    lib.qwtversion(vs, 450)
    return vs.value.decode("utf-8", errors="replace")


def fig(n: int | None = 0, flags: int = 0) -> int:
    """Create or activate a 2D figure. flags: bit 0 = log X, bit 1 = log Y."""
    lib = _ensure_started()
    return int(lib.qwtfigure(0 if n is None else int(n), int(flags)))


def spectrogram(n: int = 0, flags: int = 0) -> int:
    """Create or activate a spectrogram figure."""
    lib = _ensure_started()
    return int(lib.qwtspectrogram(int(n), int(flags)))


def spectrogram_data(
    z,
    xmin: float,
    xmax: float,
    ymin: float,
    ymax: float,
    p=None,
    t=None,
) -> int:
    """Populate the active spectrogram from a 2D [ny, nx] array or list of lists."""
    lib = _ensure_started()
    if np is not None:
        z_arr = np.ascontiguousarray(z, dtype=np.float64)
        if z_arr.ndim != 2:
            raise ValueError("z must be a 2D array with shape (ny, nx)")
        ny, nx = z_arr.shape
        z_keep: object = z_arr
        z_ptr = z_arr.ctypes.data_as(POINTER(c_double))
    else:
        rows = _to_2d_rows(z)
        ny, nx = len(rows), len(rows[0])
        flat = [val for row in rows for val in row]
        z_keep, _, z_ptr = _as_c_double_array(flat)

    if p is None and t is None:
        _ = z_keep
        return int(lib.spectrogram_info(ny, nx, ymin, ymax, xmin, xmax, z_ptr))

    p_keep, _, p_ptr = _as_c_double_array(p) if p is not None else (None, 0, POINTER(c_double)())
    t_keep, _, t_ptr = _as_c_double_array(t) if t is not None else (None, 0, POINTER(c_double)())
    _ = (z_keep, p_keep, t_keep)
    return int(lib.spectrogram_info2(ny, nx, ymin, ymax, xmin, xmax, z_ptr, p_ptr, t_ptr))


def map(n: int | None = 0) -> int:
    """Create or activate an OpenStreetMap slippy-map figure."""
    lib = _ensure_started()
    if not hasattr(lib, "qwtmap"):
        return -1
    return int(lib.qwtmap(0 if n is None else int(n)))


def xlab(label: str) -> None:
    _ensure_started().qwtxlabel(label.encode("utf-8"))


def ylab(label: str) -> None:
    _ensure_started().qwtylabel(label.encode("utf-8"))


def name(title: str) -> None:
    _ensure_started().qwttitle(title.encode("utf-8"))


def clear() -> None:
    _ensure_started().qwtclear()


def imp(status: int) -> None:
    _ensure_started().qwtsetimpstatus(int(status))


def clip_group(group: int) -> None:
    _ensure_started().qwtclipgroup(int(group))


def mw() -> None:
    _ensure_started().qwtshowmw()


def remove(line_id: int) -> None:
    _ensure_started().qwtremove(int(line_id))


def save_png(fig_id: int, filename: str | os.PathLike) -> int:
    return int(_ensure_started().qwtsave_png(int(fig_id), str(filename).encode("utf-8")))


def set_pos(fig_id: int, x: int = 0, y: int = 0, w: int = 0, h: int = 0, apply: bool = True) -> tuple[int, int, int, int]:
    cx, cy, cw, ch = c_int(x), c_int(y), c_int(w), c_int(h)
    _ensure_started().qwtsetpos(
        int(fig_id),
        ctypes.byref(cx), ctypes.byref(cy),
        ctypes.byref(cw), ctypes.byref(ch),
        1 if apply else 0,
    )
    return cx.value, cy.value, cw.value, ch.value


def set_picker_callback(callback) -> None:
    """Register callback(fig_id, line_id, index, fx, fy, x, y, z, t, legend_str)."""
    global _active_picker_cb
    lib = _ensure_started()
    if not hasattr(lib, "setcallback"):
        return

    def _trampoline(fig_id, line_id, idx, fx, fy, x, y, z, t, legend):
        label = legend.decode("utf-8", errors="replace") if legend else ""
        callback(fig_id, line_id, idx, fx, fy, x, y, z, t, label)

    _active_picker_cb = _PICKER_CB_TYPE(_trampoline)
    lib.setcallback(_active_picker_cb)


def set_clip_callback(callback) -> None:
    """Register callback(t1, t2, clip_group)."""
    global _active_clip_cb
    lib = _ensure_started()
    if not hasattr(lib, "setclipcallback"):
        return
    _active_clip_cb = _CLIP_CB_TYPE(callback)
    lib.setclipcallback(_active_clip_cb)


def plot_xy(
    x,
    y,
    name: str = "a line",
    style: str = "-b",
    width: int = 2,
    size: int = 1,
    t=None,
) -> int:
    """Plot explicit 1D x and y arrays/sequences (with optional timestamp sequence t)."""
    lib = _ensure_started()
    x_keep, nx, x_ptr = _as_c_double_array(x)
    y_keep, ny, y_ptr = _as_c_double_array(y)
    if nx == 0:
        return 0
    if ny != nx:
        raise ValueError(f"x and y lengths must match ({nx} != {ny})")

    b_name = name.encode("utf-8")
    b_style = style.encode("utf-8")
    if t is None:
        _ = (x_keep, y_keep)
        return int(lib.qwtplot(x_ptr, y_ptr, nx, b_name, b_style, int(width), int(size)))

    t_keep, nt, t_ptr = _as_c_double_array(t)
    if nt != nx:
        raise ValueError(f"t length must match x ({nt} != {nx})")
    _ = (x_keep, y_keep, t_keep)
    return int(lib.qwtplot2(x_ptr, y_ptr, nx, b_name, b_style, int(width), int(size), t_ptr))


def change_line(line_id: int, x, y, z=None, t=None) -> int:
    """Replace data arrays of an existing line in place."""
    lib = _ensure_started()
    x_keep, nx, x_ptr = _as_c_double_array(x)
    y_keep, ny, y_ptr = _as_c_double_array(y)
    if ny != nx:
        raise ValueError(f"x and y lengths must match ({nx} != {ny})")
    z_keep, _, z_ptr = _as_c_double_array(z) if z is not None else (None, 0, POINTER(c_double)())
    t_keep, _, t_ptr = _as_c_double_array(t) if t is not None else (None, 0, POINTER(c_double)())
    _ = (x_keep, y_keep, z_keep, t_keep)
    return int(lib.qwtchange(int(line_id), x_ptr, y_ptr, z_ptr, t_ptr, nx))


def pp(x):
    if np is not None:
        arr = np.asarray(x, dtype=np.float64)
        if arr.size == 0:
            return 0, None, None, None
        if arr.ndim != 2:
            raise ValueError("Expected a 2D array with 2 or 3 rows/columns")
        mp = min(arr.shape)
        if mp not in (2, 3):
            raise ValueError("Expected 2 (x, y) or 3 (x, y, t) series")
        if arr.shape[0] == mp:
            sx, sy = arr[0, :], arr[1, :]
            st = arr[2, :] if mp == 3 else None
        else:
            sx, sy = arr[:, 0], arr[:, 1]
            st = arr[:, 2] if mp == 3 else None
        return len(sx), sx, sy, st

    rows = _to_2d_rows(x)
    nrows, ncols = len(rows), len(rows[0])
    mp = min(nrows, ncols)
    if mp not in (2, 3):
        raise ValueError("Expected 2 (x, y) or 3 (x, y, t) series")
    if nrows == mp:
        sx, sy = rows[0], rows[1]
        st = rows[2] if mp == 3 else None
    else:
        sx = [r[0] for r in rows]
        sy = [r[1] for r in rows]
        st = [r[2] for r in rows] if mp == 3 else None
    return len(sx), sx, sy, st


def line(x, name: str = "a line", style: str = "-b", width: int = 2, size: int = 1) -> int:
    """Add a line from a 2xN / Nx2 (or 3xN / Nx3 with time) matrix."""
    n, sx, sy, st = pp(x)
    if n == 0:
        return 0
    return plot_xy(sx, sy, name=name, style=style, width=width, size=size, t=st)


def plot(x, name1: str = "just a plot", lines: str = "", labelX: str = "seconds", labelY: str = "") -> int:
    """Create a new figure and plot multiple series from a 2D matrix."""
    labels = [s.strip() for s in lines.split(",")] if lines else []
    colors = "bkmcrgyYG"
    fig_id = fig(0)

    if np is not None:
        arr = np.asarray(x, dtype=np.float64)
        if arr.ndim != 2 or min(arr.shape) < 2:
            raise ValueError("Expected a 2D matrix with at least 2 rows/columns")
        rows_are_series = arr.shape[0] <= arr.shape[1]
        sx = arr[0, :] if rows_are_series else arr[:, 0]
        count = arr.shape[0] if rows_are_series else arr.shape[1]
        for i in range(1, count):
            sy = arr[i, :] if rows_are_series else arr[:, i]
            lbl = labels[i - 1] if (i - 1) < len(labels) else f"item #{i}"
            col = colors[i % len(colors)]
            plot_xy(sx, sy, name=lbl, style=f"-{col}", width=2, size=3)
    else:
        rows = _to_2d_rows(x)
        nrows, ncols = len(rows), len(rows[0])
        if min(nrows, ncols) < 2:
            raise ValueError("Expected a 2D matrix with at least 2 rows/columns")
        rows_are_series = nrows <= ncols
        sx = rows[0] if rows_are_series else [r[0] for r in rows]
        count = nrows if rows_are_series else ncols
        for i in range(1, count):
            sy = rows[i] if rows_are_series else [r[i] for r in rows]
            lbl = labels[i - 1] if (i - 1) < len(labels) else f"item #{i}"
            col = colors[i % len(colors)]
            plot_xy(sx, sy, name=lbl, style=f"-{col}", width=2, size=3)

    name(name1)
    xlab(labelX)
    ylab(labelY)
    return fig_id


def test() -> int:
    """Draw a simple sine plot for quick verification."""
    fid = fig(0)
    x = [i * 10.0 / 499.0 for i in range(500)]
    y = [math.sin(v) for v in x]
    plot_xy(x, y, name="bplot 1", style="-m", width=2, size=2)
    name("the title of a plot")
    xlab("X axis label")
    ylab("Y axis label")
    return fid
