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
    selectAll();
    setHeaderData(kColId, Qt::Horizontal, "ID");
    setHeaderData(kColPath, Qt::Horizontal, "Path");
    setHeaderData(kColRating, Qt::Horizontal, "Rating");
}

void PicModel::cachedInsert(const QString& path, int rating)
{
    QSqlRecord record = this->record();
    record.setValue(kColPath, path);
    record.setValue(kColRating, rating);
    insertRecord(-1, record);
}

bool PicModel::submitInserts()
{
    bool success = submitAll();
    if (!success) {
        qDebug() << "failed to insert";
        revertAll();
    }
    selectAll();
    rowsChanged();
    return success;
}

bool PicModel::removeRows(int row, int count, const QModelIndex& parent)
{
    auto res = QSqlTableModel::removeRows(row, count, parent);
    selectAll();
    rowsChanged();
    return res;
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

void PicModel::selectAll()
{
    select();
    while (canFetchMore()) {
        fetchMore();
    }
}

} // picpic
