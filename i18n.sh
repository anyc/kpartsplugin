#!/bin/sh

xgettext --from-code=utf-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address="mailto:fischer@unix-ag.uni-kl.de"  src/*.cpp -o po/kpartsplugin.pot && sed -i -e 's/CHARSET/utf-8/;s/PACKAGE VERSION/kpartsplugin 20110708/' po/kpartsplugin.pot

for cat in $(find po -name '*.po') ; do
	msgmerge -o ${cat}.new ${cat} po/kpartsplugin.pot && mv ${cat}.new ${cat}
done
