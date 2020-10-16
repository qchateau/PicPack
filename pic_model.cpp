#include "pic_model.hpp"

#include <QBrush>
#include <QColor>
#include <QFile>
#include <QSqlRecord>

namespace picpic {

namespace {
constexpr const char* kPicturesConnectionName = "pictures";
constexpr const char* kPicturesTable = "pictures";
constexpr const char* kPicturesTableCreationQuery =
    "create table pictures ("
    "id integer primary key, "
    "path varchar(4096) unique, "
    "rating tinyint"
    ")";
}

QSqlDatabase openPicDatabase(const QString& path)
{
    if (QSqlDatabase::contains(kPicturesConnectionName)) {
        QSqlDatabase::removeDatabase(kPicturesConnectionName);
    }

    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE", kPicturesConnectionName);
    db.setDatabaseName(path);
    if (db.open()) {
        if (!db.tables().contains(kPicturesTable)) {
            QSqlQuery query(db);
            if (!query.exec(kPicturesTableCreationQuery)) {
                qDebug() << "failed to create pictures table:"
                         << query.lastError().text();
            }
            else {
                qDebug() << "created pictures table";
            }
        }
    }
    return db;
}

PicModel::PicModel(QSqlDatabase db, QObject* parent)
    : QSqlTableModel(parent, db)
{
    setTable(kPicturesTable);
    setEditStrategy(QSqlTableModel::OnFieldChange);
    setHeaderData(kColId, Qt::Horizontal, "ID");
    setHeaderData(kColPath, Qt::Horizontal, "Path");
    setHeaderData(kColRating, Qt::Horizontal, "Rating");
}

bool PicModel::insert(const QString& path, int rating)
{
    if (!match(index(0, kColPath), Qt::DisplayRole, path).empty()) {
        return true;
    }

    QSqlRecord record = this->record();
    record.setValue(kColRating, rating);
    record.setValue(kColPath, path);
    return insertRecord(-1, record);
}

QVariant PicModel::data(const QModelIndex& index, int role) const
{
    if (index.column() == kColRating && role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    else if (role == Qt::BackgroundRole) {
        QColor color = Qt::white;

        if (!QFile{index.sibling(index.row(), kColPath).data().toString()}.exists()) {
            color = QColor{255, 240, 240};
        }

        return QBrush(color);
    }
    else {
        return QSqlTableModel::data(index, role);
    }
}

void PicModel::queryChange()
{
    while (canFetchMore()) {
        fetchMore();
    }
}

} // picpic
