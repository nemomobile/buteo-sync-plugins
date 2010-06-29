DOXYGEN_BIN=doxygen

QMAKE_EXTRA_TARGETS += doc
doc.target = doc
isEmpty(DOXYGEN_BIN) {
    doc.commands = @echo "Unable to detect doxygen in PATH"
} else {
    doc.commands = @$${DOXYGEN_BIN} Doxyfile;
}
doc.depends = FORCE

# Install rules
htmldocs.files = ../doc/html/*
htmldocs.path = /${DEB_DESTDIR}/usr/share/doc/sync-app-doc
htmldocs.CONFIG += no_check_exist

INSTALLS += htmldocs    
