"""Demo script for the qwtwc Python wrapper (works with or without NumPy)."""

import math
import os
import qwtwc as q

try:
    import numpy as np  # type: ignore
except ImportError:
    np = None  # type: ignore


def run_demo() -> None:
    q.start()
    print("qwtw version:", q.version())

    if np is not None:
        x = np.linspace(0.0, 10.0, 500)
        y1 = np.sin(x) + np.cos(x * 1.8)
        y2 = np.sin(x) + np.cos(x * 0.8)
        x5 = np.linspace(0.0, 10.0, 12)
        y5 = np.sin(x5 * 0.7) + np.cos(x5 * 1.5)

        q.fig(1)
        q.line(np.stack((x, y1)))
        q.line(np.stack((x, y2)), "test #2", "-r", 3, 1)
        q.line(np.stack((x5, y5)), "test #3", " tm", 1, 8)

        m_scale = float(np.max(x)) / (2.0 * math.pi)
        x1 = np.sin(x / m_scale)
        x2 = np.cos(x / m_scale)
        x11 = x1 * 0.8 + 0.7
        x12 = x2 * 0.8 - 0.5

        q.fig(2)
        q.line(np.stack((x1, x2, x)), "test #21", "-k", 3)
        q.line(np.stack((x11, x12, x)), "test #22", "-Y", 3)
    else:
        x = [i * 10.0 / 499.0 for i in range(500)]
        y1 = [math.sin(v) + math.cos(v * 1.8) for v in x]
        y2 = [math.sin(v) + math.cos(v * 0.8) for v in x]
        x5 = [i * 10.0 / 11.0 for i in range(12)]
        y5 = [math.sin(v * 0.7) + math.cos(v * 1.5) for v in x5]

        q.fig(1)
        q.line([x, y1])
        q.line([x, y2], "test #2", "-r", 3, 1)
        q.line([x5, y5], "test #3", " tm", 1, 8)

        m_scale = 10.0 / (2.0 * math.pi)
        x1 = [math.sin(v / m_scale) for v in x]
        x2 = [math.cos(v / m_scale) for v in x]
        x11 = [v * 0.8 + 0.7 for v in x1]
        x12 = [v * 0.8 - 0.5 for v in x2]

        q.fig(2)
        q.line([x1, x2, x], "test #21", "-k", 3)
        q.line([x11, x12, x], "test #22", "-Y", 3)

    q.fig(1)
    q.xlab("seconds?")
    q.ylab("something")
    q.name("a plot 123")

    q.fig(2)
    q.xlab("meters")
    q.ylab("yards")
    q.name("a plot 234")

    p = [
        [
            i / 100.0,
            math.sin(i / 100.0),
            math.cos(i / 100.0),
            math.cos(i / 85.0) + math.sin(i / 42.0),
            math.cos(i / 75.0) + math.sin(i / 62.0) + math.cos(i / 31.0) * 0.28,
        ]
        for i in range(500)
    ]
    q.plot(p, "a bunch of plots")

    if q.map(0) >= 0:
        north = [55.688713, 55.698713, 55.678713, 55.60, 55.61]
        east = [37.901073, 37.911073, 37.905073, 37.9, 37.85]
        mt = [2.0, 4.0, 6.0, 8.8, 9.8]
        q.line([east, north, mt], "a red line on a map", "-r", 3)
        q.name("map demo")

    q.mw()


if __name__ == "__main__":
    run_demo()
    if os.environ.get("QWTW_BATCH") != "1":
        input("Press Enter to close...")
    q.close()
