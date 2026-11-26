#!/usr/bin/env python3
"""Génère les icônes SVG de ClassBoard (style trait, 24x24, couleur "currentColor").

Source unique des tracés : modifier ce fichier puis relancer
    python3 tools/icons/generate_icons.py
Les icônes sont dessinées pour ClassBoard (aucune reprise d'un jeu d'icônes tiers).
"""
import os

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "resources", "classboard")

DISCIPLINES = {
    "general": "M4 4h6v6H4zM14 4h6v6h-6zM4 14h6v6H4zM14 14h6v6h-6z",
    "math": "M5 7h5M7.5 4.5v5M14 7h5M5 15l4 4M9 15l-4 4M14 16h5M14 19h5",
    "francais": "M4 19.5A2.5 2.5 0 0 1 6.5 17H20V3H6.5A2.5 2.5 0 0 0 4 5.5zM4 19.5A2.5 2.5 0 0 0 6.5 22H20v-5",
    "histoire-geo": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18",
    "anglais": "M21 15a2 2 0 0 1-2 2H7l-4 4V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2zM8 9h8M8 13h5",
    "langues": "M4 5h7M7.5 3v2M5 9c1 3 3 5 6 6M10 5c-1 4-3 7-6 9M13 21l4-9 4 9M14.5 18h5",
    "physique-chimie": "M9 3h6M10 3v6l-5.5 9.5A2 2 0 0 0 6.2 21.5h11.6a2 2 0 0 0 1.7-3L14 9V3M7 15h10",
    "svt": "M11 20A7 7 0 0 1 4 13c0-6 6-9 16-10-1 10-4 16-10 16zM4 21c3-6 6-9 10-11",
    "technologie": "M7 6h10a1 1 0 0 1 1 1v10a1 1 0 0 1-1 1H7a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1zM9 2v4M15 2v4M9 18v4M15 18v4M2 9h4M2 15h4M18 9h4M18 15h4",
    "arts-plastiques": "M12 3a9 9 0 1 0 0 18c1.1 0 2-.9 2-2 0-.5-.2-1-.5-1.3-.3-.4-.5-.8-.5-1.3 0-1.1.9-2 2-2h2.3A4.7 4.7 0 0 0 21 9.7C21 5.9 17 3 12 3zM7 11h.01M10 7h.01M15 7h.01",
    "musique": "M9 18V5l12-2v13M3 18a3 3 0 1 0 6 0a3 3 0 1 0-6 0M15 16a3 3 0 1 0 6 0a3 3 0 1 0-6 0",
    "eps": "M8 21h8M12 17v4M7 4h10v5a5 5 0 0 1-10 0zM17 5h3v2a3 3 0 0 1-3 3M7 5H4v2a3 3 0 0 0 3 3",
    "segpa": "M3 14h8v7H3zM13 14h8v7h-8zM8 3h8v7H8z",
    "ulis": "M10 4a2 2 0 1 0 4 0a2 2 0 1 0-4 0M4 8l8 2 8-2M12 10v5M9 21l3-6 3 6",
    "autre": "M7.5 12h.01M12 12h.01M16.5 12h.01M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0",
}

ICONS = {
    "star-plus": "M11 3.5l2.4 4.9 5.4.8-3.9 3.8.9 5.4L11 15.9l-4.8 2.5.9-5.4-3.9-3.8 5.4-.8zM19 15v6M16 18h6",
    "profile": "M8 7a4 4 0 1 0 8 0a4 4 0 1 0-8 0M4 21v-1a6 6 0 0 1 6-6h4a6 6 0 0 1 6 6v1",
}


TOOLS = {
    # Palette des stylets
    "pen": "M12 20h9M16.5 3.5a2.12 2.12 0 0 1 3 3L7 19l-4 1 1-4z",
    "eraser": "M7 21l-4.3-4.3a2.4 2.4 0 0 1 0-3.4l9.6-9.6a2.4 2.4 0 0 1 3.4 0l5.6 5.6a2.4 2.4 0 0 1 0 3.4L13 21M22 21H7M5 11l9 9",
    "marker": "M9 11l-6 6v3h9l3-3M22 12l-4.6 4.6a2 2 0 0 1-2.8 0l-5.2-5.2a2 2 0 0 1 0-2.8L14 4",
    "selector": "M5 3l6.5 17 2.4-7.1L21 10.5z",
    "play": "M9 11V5a2 2 0 0 1 4 0v6M13 10V9a2 2 0 0 1 4 0v2M17 11a2 2 0 0 1 4 0v3a8 8 0 0 1-8 8h-1a7 7 0 0 1-5.6-2.8L3.6 15.6a2 2 0 0 1 3-2.6L9 15.5",
    "hand": "M18 11V6a2 2 0 0 0-4 0v5M14 10V4a2 2 0 0 0-4 0v6M10 10.5V6a2 2 0 0 0-4 0v8M18 8a2 2 0 1 1 4 0v6a8 8 0 0 1-8 8h-2c-2.8 0-4.5-.9-6-2.4l-3.6-3.6a2 2 0 0 1 2.8-2.8L7 15",
    "zoom-in": "M4 11a7 7 0 1 0 14 0a7 7 0 1 0-14 0M20 20l-3.5-3.5M8 11h6M11 8v6",
    "zoom-out": "M4 11a7 7 0 1 0 14 0a7 7 0 1 0-14 0M20 20l-3.5-3.5M8 11h6",
    "laser": "M9 12a3 3 0 1 0 6 0a3 3 0 1 0-6 0M12 2v3M12 19v3M2 12h3M19 12h3M5 5l2 2M17 17l2 2M5 19l2-2M17 7l2-2",
    "line": "M6 18L18 6M3.5 18a2.5 2.5 0 1 0 5 0a2.5 2.5 0 1 0-5 0M15.5 6a2.5 2.5 0 1 0 5 0a2.5 2.5 0 1 0-5 0",
    "text": "M4 7V5h16v2M12 5v14M9 19h6",
    "capture": "M4 8V5a1 1 0 0 1 1-1h3M16 4h3a1 1 0 0 1 1 1v3M20 16v3a1 1 0 0 1-1 1h-3M8 20H5a1 1 0 0 1-1-1v-3M9 12a3 3 0 1 0 6 0a3 3 0 1 0-6 0",
    "keyboard": "M4 6h16a1 1 0 0 1 1 1v10a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1zM7 10h.01M11 10h.01M15 10h.01M7 14h10",
    "snap": "M6 3v8a6 6 0 0 0 12 0V3M6 7h4M14 7h4M10 3v4M14 3v4",
    # Barre supérieure
    "stylus": "M12 20h9M16.5 3.5a2.12 2.12 0 0 1 3 3L7 19l-4 1 1-4z",
    "backgrounds": "M4 4h16v16H4zM4 10h16M4 15h16M10 4v16M15 4v16",
    "undo": "M9 14L4 9l5-5M4 9h10.5a5.5 5.5 0 0 1 0 11H11",
    "redo": "M15 14l5-5-5-5M20 9H9.5a5.5 5.5 0 0 0 0 11H13",
    "new-page": "M14 3H6a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V9zM14 3v6h6M12 12v6M9 15h6",
    "previous": "M15 6l-6 6 6 6",
    "next": "M9 6l6 6-6 6",
    "clear-page": "M4 7h16M10 11v6M14 11v6M6 7l1 13h10l1-13M9 7V4h6v3",
    "board": "M3 4h18v11H3zM12 15v3M8 21l4-3 4 3",
    "web": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18",
    "documents": "M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z",
    "desktop": "M4 4h16a2 2 0 0 1 2 2v9a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V6a2 2 0 0 1 2-2zM8 21h8M12 17v4",
    "menu": "M4 6h9M17 6h3M4 12h3M11 12h9M4 18h11M19 18h1M15 4v4M9 10v4M17 16v4",
    # Modes Web et Documents
    "reload": "M20 12a8 8 0 1 1-2.3-5.7M20 4v5h-5",
    "stop": "M6 6l12 12M18 6L6 18",
    "home": "M3 10l9-7 9 7v10a1 1 0 0 1-1 1h-5v-6h-6v6H4a1 1 0 0 1-1-1z",
    "tools": "M3 9h18v10a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1zM8 9V6a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v3M3 13h18M10 13v2h4v-2",
    "new-folder": "M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2zM12 10v6M9 13h6",
    "import": "M12 3v12M7 10l5 5 5-5M4 17v2a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-2",
    "export": "M12 15V3M7 8l5-5 5 5M4 17v2a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-2",
    "open": "M14 4h6v6M20 4l-9 9M18 14v5a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1V7a1 1 0 0 1 1-1h5",
    "duplicate": "M9 9h11v11H9zM5 15H4V4h11v1",
    "rename": "M4 20h4L19 9l-4-4L4 16zM13.5 6.5l4 4",
    "add": "M12 5v14M5 12h14",
    "bookmark": "M6 3h12v18l-6-4-6 4z",
    "capture-window": "M3 5h18v14H3zM3 9h18",
    # Instruments (barre de favoris)
    "ruler": "M3 16.5L16.5 3l4.5 4.5L7.5 21zM7.5 12l2 2M10.5 9l2 2M13.5 6l2 2",
    "triangle": "M4 4v16h16zM8 12v4h4z",
    "protractor": "M3 18a9 9 0 0 1 18 0zM12 18l5-6",
    "compass": "M12 3v3M10.5 7.5a1.5 1.5 0 1 0 3 0a1.5 1.5 0 1 0-3 0M11 9l-5 12M13 9l5 12M8 16h8",
    "axes": "M4 3v17h17M8 15l4-5 3 3 5-6",
    "mask": "M4 4h16v16H4zM4 4v16h9V4",
    "spotlight": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M8 12a4 4 0 1 0 8 0a4 4 0 1 0-8 0",
    "magnifier": "M4 10a6 6 0 1 0 12 0a6 6 0 1 0-12 0M20 20l-5.5-5.5",
    "timer": "M4 13a8 8 0 1 0 16 0a8 8 0 1 0-16 0M12 9v4l2.5 2.5M9.5 2h5",
    "dice": "M5 3h14a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2zM7.3 8.5a1.2 1.2 0 1 0 2.4 0a1.2 1.2 0 1 0-2.4 0M14.3 15.5a1.2 1.2 0 1 0 2.4 0a1.2 1.2 0 1 0-2.4 0M10.8 12a1.2 1.2 0 1 0 2.4 0a1.2 1.2 0 1 0-2.4 0",
    "stopwatch": "M4 13a8 8 0 1 0 16 0a8 8 0 1 0-16 0M12 13V8M10 2h4M18.5 5.5l1.5-1.5",
    "calculator": "M6 2h12a2 2 0 0 1 2 2v16a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2zM8 6h8v4H8zM8 14h.01M12 14h.01M16 14h.01M8 18h.01M12 18h.01M16 18h.01",
    "graph": "M5 3h14a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2zM6 16c2-7 4-9 6-6s4 3 6-4",
    "clock": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0-18 0M12 7v5l3 2",
}

TEMPLATE = ('<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" '
            'fill="none" stroke="currentColor" stroke-width="{w}" stroke-linecap="round" '
            'stroke-linejoin="round"><path d="{d}"/></svg>\n')


def write(folder, name, d, width=1.8):
    os.makedirs(folder, exist_ok=True)
    with open(os.path.join(folder, name + ".svg"), "w", encoding="utf-8") as f:
        f.write(TEMPLATE.format(d=d, w=width))


def main():
    for name, d in DISCIPLINES.items():
        width = 2.6 if name == "autre" else 1.8
        write(os.path.join(OUT, "disciplines"), name, d, width)
    for name, d in TOOLS.items():
        write(os.path.join(OUT, "tools"), name, d)
    for name, d in ICONS.items():
        write(os.path.join(OUT, "icons"), name, d)
    print("Icônes générées dans", OUT)


if __name__ == "__main__":
    main()
