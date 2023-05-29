#include "comboboxdelegate.h"
#include <QComboBox>
#include <QCompleter>

ComboBoxDelegate::ComboBoxDelegate(const QMap<QString, int>& leaf_paths, QObject* parent)
    : QStyledItemDelegate { parent }
    , leaf_paths { leaf_paths }
{
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto* editor = new QComboBox(parent);

    for (auto it = leaf_paths.begin(); it != leaf_paths.end(); ++it) {
        editor->addItem(it.key(), it.value());
    }

    editor->setFrame(false);
    editor->setEditable(true);
    editor->setInsertPolicy(QComboBox::NoInsert);
    editor->setCurrentIndex(-1);
    editor->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    auto completer = new QCompleter(editor->model());
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto editor_new = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(editor_new);

    for (auto it = leaf_paths.begin(); it != leaf_paths.end(); ++it) {
        editor_new->addItem(it.key(), it.value());
    }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto editor_new = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(editor_new);
    model->setData(index, editor_new->currentText());
}

void ComboBoxDelegate::ReceiveLeafPaths(const QMap<QString, int>& paths)
{
    leaf_paths = paths;
}
