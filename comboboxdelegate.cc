#include "comboboxdelegate.h"
#include <QComboBox>

ComboBoxDelegate::ComboBoxDelegate(QStringList leaf_paths, QObject* parent)
    : QStyledItemDelegate { parent }
    , leaf_paths { leaf_paths }
{
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto* editor = new QComboBox(parent);
    editor->setFrame(false);
    editor->setEditable(item_editable);
    for (int i = 0; i != leaf_paths.size(); ++i) {
        editor->addItem(leaf_paths.at(i));
    }
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto string_list = index.model()->data(index, Qt::DisplayRole).toStringList();
    auto* editor_new = static_cast<QComboBox*>(editor);
    editor_new->addItems(string_list);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::SetItem(QStringList list, bool editable)
{
    leaf_paths = list;
    item_editable = editable;
}
