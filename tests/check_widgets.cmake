# Contrôle des applications web ClassBoard de la bibliothèque.
# Usage : cmake -DLIBRARY_DIR=<resources/library/applications> -P check_widgets.cmake

set(CLASSBOARD_WIDGETS Minuteur.wgt TirageAuSort.wgt)

foreach(widget IN LISTS CLASSBOARD_WIDGETS)
    set(dir "${LIBRARY_DIR}/${widget}")

    foreach(file config.xml index.html icon.png)
        if(NOT EXISTS "${dir}/${file}")
            message(FATAL_ERROR "${widget} : fichier manquant ${file}")
        endif()
    endforeach()

    file(READ "${dir}/config.xml" config)
    if(NOT config MATCHES "<widget" OR NOT config MATCHES "<content src=\"index.html\"")
        message(FATAL_ERROR "${widget} : config.xml invalide")
    endif()

    file(READ "${dir}/index.html" html)
    if(html MATCHES "https?://")
        message(FATAL_ERROR "${widget} : référence réseau interdite (fonctionnement hors connexion requis)")
    endif()

    message(STATUS "${widget} : OK")
endforeach()
