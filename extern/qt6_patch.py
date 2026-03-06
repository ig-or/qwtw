#!/usr/bin/env python3
"""Patch Qwt 6.3.0 source for Qt 6.10 compatibility.

Qt 6.5+ removed many deprecated APIs that Qwt 6.3.0 still uses.
This script applies all necessary source-level fixes.
"""

import os
import re
import sys

QWT_SRC = os.path.join(os.path.dirname(__file__), "qwt", "src")

def patch_file(path, replacements):
    """Apply a list of (old, new) replacements to a file."""
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
    original = content
    for old, new in replacements:
        if old not in content:
            print(f"  WARNING: pattern not found in {os.path.basename(path)}: {old[:60]}...")
            continue
        content = content.replace(old, new)
    if content != original:
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"  patched {os.path.basename(path)}")
    return content != original

def patch_regex(path, pattern, replacement, flags=0):
    """Apply a regex replacement to a file."""
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
    new_content = re.sub(pattern, replacement, content, flags=flags)
    if new_content != content:
        with open(path, "w", encoding="utf-8") as f:
            f.write(new_content)
        return True
    return False


def fix_qvector_forward_declarations():
    """Replace 'template <typename T> class QVector;' with #include <QVector>."""
    print("\n[1] Fixing QVector forward declarations...")
    files = [
        "qwt_spline.h", "qwt_plot_picker.h", "qwt_plot_histogram.h",
        "qwt_matrix_raster_data.h", "qwt_color_map.h", "qwt_clipper.h",
    ]
    for fname in files:
        path = os.path.join(QWT_SRC, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            content = f.read()
        # Replace the forward declaration with nothing (QVector is available via Qt headers)
        new = re.sub(
            r'template\s*<\s*typename\s+T\s*>\s*class\s+QVector\s*;',
            '',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qwheelevent():
    """Replace QWheelEvent::delta() -> angleDelta().y() and ::pos() -> position().toPoint()."""
    print("\n[2] Fixing QWheelEvent deprecated API...")
    files = [
        "qwt_wheel.cpp", "qwt_slider.cpp", "qwt_scale_draw.cpp",
        "qwt_plot_renderer.cpp", "qwt_picker.cpp", "qwt_panner.cpp",
        "qwt_magnifier.cpp", "qwt_dial.cpp", "qwt_counter.cpp",
        "qwt_abstract_slider.cpp",
    ]
    for fname in files:
        path = os.path.join(QWT_SRC, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            content = f.read()
        original = content
        # event->delta() -> event->angleDelta().y()
        content = re.sub(r'(\w+)->delta\(\)', r'\1->angleDelta().y()', content)
        # event->pos() in context of QWheelEvent -> event->position().toPoint()
        # Be careful: only replace when 'event' is likely a QWheelEvent
        content = re.sub(r'(event)->pos\(\)', r'\1->position().toPoint()', content)
        if content != original:
            with open(path, "w") as f:
                f.write(content)
            print(f"  patched {fname}")


def fix_qstyleoption_init():
    """Replace QStyleOption::init(widget) -> initFrom(widget)."""
    print("\n[3] Fixing QStyleOption::init() -> initFrom()...")
    files = [
        "qwt_wheel.cpp", "qwt_thermo.cpp", "qwt_text_label.cpp",
        "qwt_slider.cpp", "qwt_scale_widget.cpp",
        "qwt_plot_abstract_canvas.cpp", "qwt_plot_canvas.cpp",
        "qwt_legend_label.cpp", "qwt_knob.cpp", "qwt_dial.cpp",
        "qwt_arrow_button.cpp", "qwt_counter.cpp",
    ]
    for fname in files:
        path = os.path.join(QWT_SRC, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            content = f.read()
        # Pattern: option.init( widget ) or styleOption.init( this )
        new = re.sub(r'(\w+)\.init\(\s*', r'\1.initFrom( ', content)
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_global_strut():
    """Remove QApplication::globalStrut() calls."""
    print("\n[4] Fixing QApplication::globalStrut() removal...")
    # Search all .cpp files
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "globalStrut" not in content:
            continue
        # Pattern: hint.expandedTo( QApplication::globalStrut() )  -> hint
        # or similar: return QSize(...).expandedTo(QApplication::globalStrut())
        new = re.sub(
            r'\.expandedTo\(\s*QApplication::globalStrut\(\)\s*\)',
            '',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qpainterpath_include():
    """Add #include <QPainterPath> to qwt_global.h for Qt6."""
    print("\n[5] Adding QPainterPath include to qwt_global.h...")
    path = os.path.join(QWT_SRC, "qwt_global.h")
    patch_file(path, [
        ('#include <qglobal.h>\n',
         '#include <qglobal.h>\n#include <QPainterPath>\n'),
    ])


def fix_qlinef_intersect():
    """Replace QLineF::intersect() -> intersects()."""
    print("\n[6] Fixing QLineF::intersect() -> intersects()...")
    # Only patch qwt_compass_rose.cpp which uses QLineF::intersect().
    # qwt_raster_data.cpp has a custom ContourPlane::intersect() that must NOT be renamed.
    for fname in ["qwt_compass_rose.cpp"]:
        path = os.path.join(QWT_SRC, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            content = f.read()
        if ".intersect(" not in content:
            continue
        new = content.replace(".intersect(", ".intersects(")
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qfontmetrics_width():
    """Replace QFontMetrics::width() -> horizontalAdvance()."""
    print("\n[7] Fixing QFontMetrics::width() -> horizontalAdvance()...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if not re.search(r'(?:fontMetrics\(\)|[fF]m)\s*[\.\)]\s*width\s*\(', content):
            continue
        # fontMetrics().width(...) -> fontMetrics().horizontalAdvance(...)
        new = re.sub(r'(fontMetrics\(\))\s*\.\s*width\s*\(', r'\1.horizontalAdvance(', content)
        # fm.width(...) -> fm.horizontalAdvance(...)
        new = re.sub(r'(\bfm)\s*\.\s*width\s*\(', r'\1.horizontalAdvance(', new)
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_setmargin():
    """Replace QLayout::setMargin(n) -> setContentsMargins(n,n,n,n)."""
    print("\n[8] Fixing QLayout::setMargin() -> setContentsMargins()...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "setMargin(" not in content:
            continue
        new = re.sub(
            r'(\w+)->setMargin\(\s*(\w+)\s*\)',
            r'\1->setContentsMargins( \2, \2, \2, \2 )',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qgl_widget():
    """Fix qwt_plot_glcanvas to use QOpenGLWidget instead of QGLWidget."""
    print("\n[9] Fixing QGLWidget -> QOpenGLWidget in qwt_plot_glcanvas...")

    # Header
    hpath = os.path.join(QWT_SRC, "qwt_plot_glcanvas.h")
    if os.path.exists(hpath):
        with open(hpath, "r") as f:
            content = f.read()
        replacements = [
            ('#include <qgl.h>', '#include <QOpenGLWidget>'),
            ('class QWT_EXPORT QwtPlotGLCanvas: public QGLWidget, public QwtPlotAbstractGLCanvas',
             'class QWT_EXPORT QwtPlotGLCanvas: public QOpenGLWidget, public QwtPlotAbstractGLCanvas'),
            ('explicit QwtPlotGLCanvas( const QGLFormat &, QwtPlot * = NULL );', ''),
        ]
        for old, new in replacements:
            content = content.replace(old, new)
        with open(hpath, "w") as f:
            f.write(content)
        print(f"  patched qwt_plot_glcanvas.h")

    # Source
    cpath = os.path.join(QWT_SRC, "qwt_plot_glcanvas.cpp")
    if os.path.exists(cpath):
        with open(cpath, "r") as f:
            content = f.read()
        content = content.replace('#include <qgl.h>', '#include <QOpenGLWidget>')
        content = content.replace('#include <qglformat.h>', '')
        content = re.sub(r'QGLWidget', 'QOpenGLWidget', content)
        # Remove the QGLFormat constructor overload implementation
        content = re.sub(
            r'QwtPlotGLCanvas::QwtPlotGLCanvas\(\s*const QGLFormat\s*&[^}]+\}',
            '',
            content,
            flags=re.DOTALL
        )
        with open(cpath, "w") as f:
            f.write(content)
        print(f"  patched qwt_plot_glcanvas.cpp")


def fix_getcontentsmargins():
    """Replace getContentsMargins(&l,&t,&r,&b) with QMargins-based code."""
    print("\n[11] Fixing getContentsMargins() removal...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "getContentsMargins" not in content:
            continue

        directions = ['left', 'top', 'right', 'bottom']

        def replace_gcm(m):
            full_prefix = m.group(1) or ""
            args = m.group(2)
            parts = [a.strip().lstrip('&') for a in args.split(',')]
            obj_expr = full_prefix.replace("getContentsMargins", "contentsMargins")
            if not obj_expr:
                obj_expr = "contentsMargins"
            else:
                obj_expr += "contentsMargins"
            lines = [f"{{ const QMargins _cm = {obj_expr}();"]
            for i, p in enumerate(parts):
                if p in ('NULL', 'nullptr', '0') or i >= 4:
                    continue
                lines.append(f" {p} = _cm.{directions[i]}();")
            lines.append(" }")
            return "".join(lines)

        # Match the full prefix chain including chained -> and () calls
        new = re.sub(
            r'((?:[\w.>\(\)]+(?:->|\.))*)getContentsMargins\(\s*([^)]+)\s*\)',
            replace_gcm,
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qlayout_margin():
    """Fix QLayout::margin() -> contentsMargins().left() for QwtDynGridLayout."""
    print("\n[12] Fixing QLayout::margin() -> contentsMargins().left()...")
    path = os.path.join(QWT_SRC, "qwt_legend.cpp")
    if os.path.exists(path):
        with open(path, "r") as f:
            content = f.read()
        # tl->margin() where tl is QwtDynGridLayout* (inherits QLayout)
        # Only match tl->margin(), not label->margin() or this->margin() (which are Qwt's own)
        new = content.replace("tl->margin()", "tl->contentsMargins().left()")
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched qwt_legend.cpp")


def fix_qfont_resolve():
    """Fix QFont::resolve(uint) -> setResolveMask()."""
    print("\n[13] Fixing QFont::resolve(AllPropertiesResolved)...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "AllPropertiesResolved" not in content:
            continue
        new = re.sub(
            r'(\w+)\.resolve\(\s*QFont::AllPropertiesResolved\s*\)',
            r'\1.setResolveMask( QFont::AllPropertiesResolved )',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_missing_includes():
    """Add missing includes needed in Qt6."""
    print("\n[10] Adding missing includes for Qt6...")

    # qwt_painter.cpp needs QPainterPath
    path = os.path.join(QWT_SRC, "qwt_painter.cpp")
    if os.path.exists(path):
        with open(path, "r") as f:
            content = f.read()
        if "QPainterPath" not in content or "#include" not in content.split("QPainterPath")[0][-200:]:
            if '#include "qwt_painter.h"' in content:
                content = content.replace(
                    '#include "qwt_painter.h"',
                    '#include "qwt_painter.h"\n#include <QPainterPath>'
                )
                with open(path, "w") as f:
                    f.write(content)
                print(f"  patched qwt_painter.cpp (QPainterPath)")

    # qwt_plot_abstract_canvas.cpp may need QPainterPath
    path = os.path.join(QWT_SRC, "qwt_plot_abstract_canvas.cpp")
    if os.path.exists(path):
        with open(path, "r") as f:
            content = f.read()
        if "QPainterPath" in content and "#include <QPainterPath>" not in content:
            content = content.replace(
                '#include "qwt_plot_abstract_canvas.h"',
                '#include "qwt_plot_abstract_canvas.h"\n#include <QPainterPath>'
            )
            with open(path, "w") as f:
                f.write(content)
            print(f"  patched qwt_plot_abstract_canvas.cpp")


def fix_qdesktopwidget():
    """Replace QDesktopWidget/QApplication::desktop() with QScreen in qwt_painter.cpp."""
    print("\n[14] Fixing QDesktopWidget -> QScreen in qwt_painter.cpp...")
    path = os.path.join(QWT_SRC, "qwt_painter.cpp")
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        content = f.read()
    changed = False
    if "<qdesktopwidget.h>" in content:
        content = content.replace(
            "#include <qdesktopwidget.h>",
            "#include <QScreen>\n#include <QGuiApplication>"
        )
        changed = True
    if "QDesktopWidget" in content:
        content = content.replace(
            "QDesktopWidget *desktop = QApplication::desktop();\n"
            "        if ( desktop )\n"
            "        {\n"
            "            screenResolution.setWidth( desktop->logicalDpiX() );\n"
            "            screenResolution.setHeight( desktop->logicalDpiY() );\n"
            "        }",
            "if ( const QScreen *screen = QGuiApplication::primaryScreen() )\n"
            "        {\n"
            "            screenResolution.setWidth( qRound( screen->logicalDotsPerInchX() ) );\n"
            "            screenResolution.setHeight( qRound( screen->logicalDotsPerInchY() ) );\n"
            "        }"
        )
        changed = True
    if changed:
        with open(path, "w") as f:
            f.write(content)
        print(f"  patched qwt_painter.cpp")


def fix_midbutton():
    """Replace Qt::MidButton -> Qt::MiddleButton."""
    print("\n[15] Fixing Qt::MidButton -> Qt::MiddleButton...")
    path = os.path.join(QWT_SRC, "qwt_event_pattern.cpp")
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        content = f.read()
    new = content.replace("Qt::MidButton", "Qt::MiddleButton")
    if new != content:
        with open(path, "w") as f:
            f.write(new)
        print(f"  patched qwt_event_pattern.cpp")


def fix_qlayout_setmargin():
    """Replace QLayout::setMargin(n) -> setContentsMargins(n,n,n,n) in qwt_dyngrid_layout.cpp."""
    print("\n[16] Fixing QLayout::setMargin() in qwt_dyngrid_layout.cpp...")
    path = os.path.join(QWT_SRC, "qwt_dyngrid_layout.cpp")
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        content = f.read()
    new = content.replace(
        "setMargin( margin );",
        "setContentsMargins( margin, margin, margin, margin );"
    )
    if new != content:
        with open(path, "w") as f:
            f.write(new)
        print(f"  patched qwt_dyngrid_layout.cpp")


def fix_qwt_painter_init():
    """Fix QStyleOption::init() -> initFrom() in qwt_painter.cpp."""
    print("\n[17] Fixing QStyleOption::init() in qwt_painter.cpp...")
    path = os.path.join(QWT_SRC, "qwt_painter.cpp")
    if not os.path.exists(path):
        return
    patch_file(path, [
        ("opt.init( widget );", "opt.initFrom( widget );"),
    ])


def fix_qstring_sprintf():
    """Replace QString::sprintf() -> QString::asprintf()."""
    print("\n[18] Fixing QString::sprintf() -> asprintf()...")
    for fname in ["qwt_picker.cpp", "qwt_plot_picker.cpp"]:
        path = os.path.join(QWT_SRC, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            content = f.read()
        new = re.sub(
            r'(\w+)\.sprintf\(\s*',
            r'\1 = QString::asprintf( ',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_wheelevent_pos():
    """Replace wheelEvent->pos() -> wheelEvent->position().toPoint()."""
    print("\n[19] Fixing QWheelEvent::pos()...")
    path = os.path.join(QWT_SRC, "qwt_picker.cpp")
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        content = f.read()
    new = content.replace("wheelEvent->pos()", "wheelEvent->position().toPoint()")
    if new != content:
        with open(path, "w") as f:
            f.write(new)
        print(f"  patched qwt_picker.cpp")


def fix_highquality_antialiasing():
    """Remove QPainter::HighQualityAntialiasing (removed in Qt6)."""
    print("\n[20] Fixing QPainter::HighQualityAntialiasing...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "HighQualityAntialiasing" not in content:
            continue
        new = re.sub(
            r'\s*painter->setRenderHint\(\s*QPainter::HighQualityAntialiasing[^;]*;\n?',
            '\n',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_noncosmeticdefaultpen():
    """Remove QPainter::NonCosmeticDefaultPen (removed in Qt6)."""
    print("\n[21] Fixing QPainter::NonCosmeticDefaultPen...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "NonCosmeticDefaultPen" not in content:
            continue
        new = re.sub(
            r'\s*painter->setRenderHint\(\s*QPainter::NonCosmeticDefaultPen[^;]*;\n?',
            '\n',
            content
        )
        new = re.sub(
            r'if \( hints\.testFlag\(\s*QPainter::NonCosmeticDefaultPen\s*\) \)\s*\n\s*\w+ = \w+;',
            '',
            new
        )
        new = re.sub(
            r'const QPainter::RenderHints hints = painter->renderHints\(\);\s*\n\s*if \( hints\.testFlag\( QPainter::NonCosmeticDefaultPen \) \)\s*\n\s*scalablePen = true;',
            'scalablePen = true;',
            new
        )
        new = re.sub(
            r'QPainter::RenderHints hints = painter->renderHints\(\);\s*\n\s*if \( hints\.testFlag\( QPainter::NonCosmeticDefaultPen \) \)\s*\n\s*isCosmetic = false;',
            'isCosmetic = false;',
            new
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qvariantsetvalue():
    """Replace qVariantSetValue(var, val) -> var = QVariant::fromValue(val)."""
    print("\n[22] Fixing qVariantSetValue()...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "qVariantSetValue" not in content:
            continue
        def replace_qvs(m):
            var = m.group(1).strip()
            val = m.group(2).strip()
            return f"{var} = QVariant::fromValue( {val} )"
        new = re.sub(
            r'qVariantSetValue\(\s*(\w+)\s*,\s*(.+?)\s*\)',
            replace_qvs,
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qapplication_desktop():
    """Remove QApplication::desktop() as QPaintDevice* arg to QFont."""
    print("\n[23] Fixing QApplication::desktop() in QFont constructors...")
    for fname in ["qwt_text.cpp", "qwt_painter.cpp"]:
        path = os.path.join(QWT_SRC, fname)
        if not os.path.exists(path):
            continue
        with open(path, "r") as f:
            content = f.read()
        changed = False
        if "#include <qdesktopwidget.h>" in content:
            content = content.replace("#include <qdesktopwidget.h>", "")
            changed = True
        if "QApplication::desktop()" in content:
            content = content.replace(
                ", QApplication::desktop()",
                ""
            )
            changed = True
        if changed:
            with open(path, "w") as f:
                f.write(content)
            print(f"  patched {fname}")


def fix_allpropertiesresolved():
    """Replace QFont::AllPropertiesResolved with 0xFFFFFFFF."""
    print("\n[24] Fixing QFont::AllPropertiesResolved...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "AllPropertiesResolved" not in content:
            continue
        new = content.replace("QFont::AllPropertiesResolved", "0xFFFFFFFF")
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qflags_default():
    """Fix QFlags = 0x00 default parameter."""
    print("\n[25] Fixing QFlags default parameter = 0x00 -> {}...")
    path = os.path.join(QWT_SRC, "qwt_plot_layout.h")
    if os.path.exists(path):
        patch_file(path, [("Options options = 0x00", "Options options = {}")])


def fix_qmatrix():
    """Remove QMatrix usage (replaced by QTransform in Qt6)."""
    print("\n[26] Fixing QMatrix -> QTransform...")
    path = os.path.join(QWT_SRC, "qwt_painter_command.h")
    if os.path.exists(path):
        patch_file(path, [("        QMatrix matrix;\n", "")])


def fix_postscript_engine():
    """Remove QPaintEngine::PostScript case (removed in Qt6)."""
    print("\n[27] Fixing QPaintEngine::PostScript removal...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "QPaintEngine::PostScript" not in content:
            continue
        new = content.replace(
            "            case QPaintEngine::PostScript:\n",
            ""
        )
        new = new.replace(
            "            case QPaintEngine::MacPrinter:\n",
            ""
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qtconcurrent_run():
    """Fix QtConcurrent::run() member function syntax for Qt6."""
    print("\n[28] Fixing QtConcurrent::run() syntax...")
    path = os.path.join(QWT_SRC, "qwt_plot_spectrogram.cpp")
    if not os.path.exists(path):
        return
    patch_file(path, [
        ("QtConcurrent::run(\n                this, &QwtPlotSpectrogram::renderTile,",
         "QtConcurrent::run(\n                &QwtPlotSpectrogram::renderTile, this,"),
    ])


def fix_setpagesizemm():
    """Replace QPdfWriter::setPageSizeMM() with setPageSize()."""
    print("\n[29] Fixing QPdfWriter::setPageSizeMM()...")
    path = os.path.join(QWT_SRC, "qwt_plot_renderer.cpp")
    if not os.path.exists(path):
        return
    patch_file(path, [
        ("pdfWriter.setPageSizeMM( sizeMM );",
         "pdfWriter.setPageSize( QPageSize( sizeMM, QPageSize::Millimeter ) );"),
    ])


def fix_qtime_timer():
    """Replace QTime timer usage with QElapsedTimer."""
    print("\n[30] Fixing QTime -> QElapsedTimer in qwt_wheel.cpp...")
    path = os.path.join(QWT_SRC, "qwt_wheel.cpp")
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        content = f.read()
    changed = False
    if "#include <qelapsedtimer.h>" not in content and "QTime time;" in content:
        content = content.replace(
            "#include <qdatetime.h>",
            "#include <qdatetime.h>\n#include <qelapsedtimer.h>"
        )
        content = content.replace("QTime time;", "QElapsedTimer time;")
        changed = True
    if changed:
        with open(path, "w") as f:
            f.write(content)
        print(f"  patched qwt_wheel.cpp")


def fix_qfontmetrics_width_all():
    """Fix remaining QFontMetrics::width() calls the main function missed."""
    print("\n[31] Fixing remaining QFontMetrics::width()...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        new = re.sub(
            r'(QFontMetrics\([^)]+\))\s*\.\s*width\s*\(',
            r'\1.horizontalAdvance(',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def fix_qlayout_margin_method():
    """Replace QLayout::margin() -> contentsMargins().left() in qwt_dyngrid_layout.cpp."""
    print("\n[32] Fixing QLayout::margin() method calls...")
    path = os.path.join(QWT_SRC, "qwt_dyngrid_layout.cpp")
    if not os.path.exists(path):
        return
    with open(path, "r") as f:
        content = f.read()
    # Only replace bare margin() calls (not setMargin, not ->margin on Qwt widgets)
    new = re.sub(r'(?<!\w)margin\(\)', 'contentsMargins().left()', content)
    if new != content:
        with open(path, "w") as f:
            f.write(new)
        print(f"  patched qwt_dyngrid_layout.cpp")


def fix_qflags_assign_zero():
    """Fix d_data->layoutFlags = 0 for QFlags in Qt6."""
    print("\n[33] Fixing QFlags = 0 assignments...")
    path = os.path.join(QWT_SRC, "qwt_scale_widget.cpp")
    if os.path.exists(path):
        patch_file(path, [("d_data->layoutFlags = 0;", "d_data->layoutFlags = {};")])


def fix_moc_incomplete_types():
    """Add full type includes to headers that forward-declare MOC-needed types."""
    print("\n[35] Fixing incomplete types for MOC output...")
    path = os.path.join(QWT_SRC, "qwt_abstract_legend.h")
    if os.path.exists(path):
        with open(path, "r") as f:
            content = f.read()
        if 'class QwtLegendData;' in content:
            content = content.replace(
                '#include "qwt_global.h"\n#include <qframe.h>\n\nclass QwtLegendData;',
                '#include "qwt_global.h"\n#include "qwt_legend_data.h"\n#include <qframe.h>\n'
            )
            with open(path, "w") as f:
                f.write(content)
            print(f"  patched qwt_abstract_legend.h")

    path = os.path.join(QWT_SRC, "qwt_picker.h")
    if os.path.exists(path):
        with open(path, "r") as f:
            content = f.read()
        if 'class QPen;' in content and '#include <QPen>' not in content:
            content = content.replace('class QPen;\nclass QFont;', '#include <QPen>\n#include <QFont>')
            with open(path, "w") as f:
                f.write(content)
            print(f"  patched qwt_picker.h")


def fix_getcontentsmargins_broken():
    """Fix getContentsMargins replacements that the regex mangled."""
    print("\n[34] Fixing broken getContentsMargins patterns...")
    for fname in os.listdir(QWT_SRC):
        if not fname.endswith(".cpp"):
            continue
        path = os.path.join(QWT_SRC, fname)
        with open(path, "r") as f:
            content = f.read()
        if "->{ const QMargins _cm =" not in content:
            continue
        # Fix pattern: obj->{ const QMargins ... }; -> { const QMargins _cm = obj->contentsMargins(); ... }
        new = re.sub(
            r'(\w+(?:\(\))?)->\{\s*const QMargins _cm = contentsMargins\(\);(.*?)\}',
            r'{ const QMargins _cm = \1->contentsMargins();\2}',
            content
        )
        if new != content:
            with open(path, "w") as f:
                f.write(new)
            print(f"  patched {fname}")


def main():
    print(f"Patching Qwt source in: {QWT_SRC}")

    fix_qvector_forward_declarations()
    fix_qwheelevent()
    fix_qstyleoption_init()
    fix_global_strut()
    fix_qpainterpath_include()
    fix_qlinef_intersect()
    fix_qfontmetrics_width()
    fix_setmargin()
    fix_qgl_widget()
    fix_missing_includes()
    fix_getcontentsmargins()
    fix_qlayout_margin()
    fix_qfont_resolve()
    fix_qdesktopwidget()
    fix_midbutton()
    fix_qlayout_setmargin()
    fix_qwt_painter_init()
    fix_qstring_sprintf()
    fix_wheelevent_pos()
    fix_highquality_antialiasing()
    fix_noncosmeticdefaultpen()
    fix_qvariantsetvalue()
    fix_qapplication_desktop()
    fix_allpropertiesresolved()
    fix_qflags_default()
    fix_qmatrix()
    fix_postscript_engine()
    fix_qtconcurrent_run()
    fix_setpagesizemm()
    fix_qtime_timer()
    fix_qfontmetrics_width_all()
    fix_qlayout_margin_method()
    fix_qflags_assign_zero()
    fix_moc_incomplete_types()
    fix_getcontentsmargins_broken()

    print("\nDone! All Qt6.10 compatibility patches applied.")


if __name__ == "__main__":
    main()
