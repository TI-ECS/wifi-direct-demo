#ifndef _TYPES_H_
#define _TYPES_H_

#define REGISTER_TYPES()                                        \
    do {                                                        \
        qRegisterMetaType<QStringMap>("QStringMap");            \
        qDBusRegisterMetaType<QStringMap>();                    \
        qRegisterMetaType<QVariantMapList>("QVariantMapList");  \
        qDBusRegisterMetaType<QVariantMapList>();               \
        qRegisterMetaType<QStringMapList>("QStringMapList");    \
        qDBusRegisterMetaType<QStringMapList>();                \
    } while (0);

typedef QMap<QString, QString> QStringMap;
Q_DECLARE_METATYPE(QStringMap)

typedef QList<QStringMap> QStringMapList;
Q_DECLARE_METATYPE(QStringMapList)

typedef QList<QVariantMap> QVariantMapList;
Q_DECLARE_METATYPE(QVariantMapList)



#endif /* _TYPES_H_ */
