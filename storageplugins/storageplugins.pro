TEMPLATE = subdirs

hcalendar.subdir = hcalendar
hcalendar.target = sub-hcalendar

hcalendar_tests.subdir = hcalendar/unittest
hcalendar_tests.target = sub-hcalendar-tests
hcalendar_tests.depends = sub-hcalendar

hcontacts.subdir = hcontacts
hcontacts.target = sub-hcontacts

hcontacts_tests.subdir = hcontacts/unittest
hcontacts_tests.target = sub-hcontacts-tests
hcontacts_tests.depends = sub-hcontacts

hnotes.subdir = hnotes
hnotes.target = sub-hnotes

hnotes_tests.subdir = hnotes/unittest
hnotes_tests.target = sub-hnotes-tests
hnotes_tests.depends = sub-hnotes

SUBDIRS += \
    hcalendar \
    hcalendar_tests \
    hcontacts \
    hcontacts_tests \
    hnotes \
    hnotes_tests

