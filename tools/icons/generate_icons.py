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
    for name, d in ICONS.items():
        write(os.path.join(OUT, "icons"), name, d)
    print("Icônes générées dans", OUT)


if __name__ == "__main__":
    main()
