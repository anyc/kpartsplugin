KPartsPlugin

written by Thomas Fischer <fischer@unix-ag.uni-kl.de>
released under the GNU General Public License version 3 or later

based on QtBrowserPlugin example by Trolltech/Nokia Cooperation
released under the 3-clause BSD license
http://qt.nokia.com/products/appdev/add-on-products/catalog/4/Utilities/qtbrowserplugin
http://doc.qt.nokia.com/solutions/4/qtbrowserplugin/

Please see the header of each source file (.cpp, .h) which license applies. All other files (e.g. documentation) are released under the GNU GPL v3 or later.

This software implements a plugin for Netscape-compatible browsers in a Unix environment.
This plugin uses KDE's KParts technology to embed file viewers (e.g. for PDF files) into non-KDE browsers.
Tested browsers include both Mozilla Firefox, Chrome, and Opera.

You can control the installation directory by defining the variable
NSPLUGIN_INSTALL_DIR when calling cmake.
Example:
 cmake -DNSPLUGIN_INSTALL_DIR=/usr/lib128/nsbrowser/plugins/
By default, the plugin will get installed to
/usr/lib/nsbrowser/plugins/ on 32-bit architectures and
/usr/lib64/nsbrowser/plugins/ on 64-bit architectures.
