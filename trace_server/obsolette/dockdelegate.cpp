/*#include <QStyledItemDelegate>
class DockedTreeDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:

    explicit DockedTreeDelegate (QObject * parent = 0, QPixmap const & icon = QPixmap());
    QPoint calcIconPos (QStyleOptionViewItem const & option) const;
    void paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const;
    QSize sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const;
    bool editorEvent (QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index);

signals:
    void closeIndexClicked (QModelIndex const &);

protected:
    QPixmap m_icon;
    static const int margin = 2; // pixels to keep arount the icon

    Q_DISABLE_COPY(DockedTreeDelegate)
};


class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    void setEditorData(QWidget *editor, QModelIndex const &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const;
    void updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const;
};*/

/*	DockedTreeDelegate::DockedTreeDelegate (QObject * parent, QPixmap const & icon)
		: QStyledItemDelegate(parent)
		, m_icon(icon)
	{
		if (m_icon.isNull())
		{
			m_icon = qApp->style()->standardPixmap(QStyle::SP_DialogCloseButton);
		}
	}

	QPoint DockedTreeDelegate::calcIconPos (QStyleOptionViewItem const & option) const
	{
		return QPoint(option.rect.right() - m_icon.width() - margin,
					  option.rect.center().y() - m_icon.height()/2);
	}

	void DockedTreeDelegate::paint (QPainter * painter, QStyleOptionViewItem const & option, QModelIndex const & index) const
	{
		TreeView * t = static_cast<TreeView *>(parent());
		DockTreeModel * m = static_cast<DockTreeModel *>(t->model());

		TreeModel<DockedInfo>::node_t const * n = m->getItemFromIndex(index);

		int const col = index.column();
		if (col == e_InCentralWidget)
		{
			if (m->data(index, Qt::DisplayRole).toBool())
				painter->drawPixmap(calcIconPos(option), m_icon);
		}
		QStyledItemDelegate::paint(painter, option, index);
		// Only display the close icon for top level items...
		//if(!index.parent().isValid()
				// ...and when the mouse is hovering the item
				// (mouseTracking must be enabled on the view)
				//&& (option.state & QStyle::State_MouseOver))
				//)
	}

	QSize DockedTreeDelegate::sizeHint (QStyleOptionViewItem const & option, QModelIndex const & index) const
	{
		QSize size = QStyledItemDelegate::sizeHint(option, index);

		// Make some room for the close icon
		if (!index.parent().isValid()) {
			size.rwidth() += m_icon.width() + margin * 2;
			size.setHeight(qMax(size.height(), m_icon.height() + margin * 2));
		}
		return size;
	}

	bool DockedTreeDelegate::editorEvent (QEvent * event, QAbstractItemModel * model, QStyleOptionViewItem const & option, QModelIndex const & index)
	{
		// Emit a signal when the icon is clicked
		if (!index.parent().isValid() && event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent const * mouseEvent = static_cast<QMouseEvent const *>(event);
			QRect const closeButtonRect = m_icon.rect().translated(calcIconPos(option));
			if (closeButtonRect.contains(mouseEvent->pos()))
			{
				emit closeIndexClicked(index);
			}
		}
		return false;
	}

	SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
		: QStyledItemDelegate(parent)
	{
	}

	QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
		const QStyleOptionViewItem &,
		const QModelIndex &) const
	{
		QSpinBox *editor = new QSpinBox(parent);
		editor->setFrame(false);
		editor->setMinimum(0);
		editor->setMaximum(100);

		return editor;
	}

	void SpinBoxDelegate::setEditorData(QWidget *editor,
										const QModelIndex &index) const
	{
		int value = index.model()->data(index, Qt::EditRole).toInt();

		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		spinBox->setValue(value);
	}

	void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
									   const QModelIndex &index) const
	{
		QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
		spinBox->interpretText();
		int value = spinBox->value();

		model->setData(index, value, Qt::EditRole);
	}

	void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &) const
	{
		editor->setGeometry(option.rect);
	}

*/
