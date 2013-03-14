#ifndef QT_CLIST_GLOBAL_H
#define QT_CLIST_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QT_CLIST_LIBRARY)
#  define QT_CLISTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QT_CLISTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QT_CLIST_GLOBAL_H
