TEMPLATE = subdirs

linux-g++-maemo {
	message("Compiling for harmattan")
	SUBDIRS += \
	           hnotes  \
        	   hnotes/unittest  \
	           hcalendar \
	           hcalendar/unittest
}

SUBDIRS += \
           hcontacts \
           hcontacts/unittest
