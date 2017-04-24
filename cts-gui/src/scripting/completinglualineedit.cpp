#include <cts-gui/scripting/completinglualineedit.h>
#include <cts-gui/tools/luaitemmodel.h>

#include <QtGui/QKeyEvent>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QDirModel>
#include <QtWidgets/QScrollBar>

namespace cts {	namespace gui {

	LuaCompleter::LuaCompleter(sol::state& luaVmState, QWidget* parent)
		: QCompleter(parent)
		, _luaVmState(luaVmState)
		, m_model(nullptr)
	{
		m_model = new LuaItemModel(this);
		m_model->setData(&_luaVmState, LuaTreeItem::CompleterModel);
		setModel(m_model);
	}


	QStringList LuaCompleter::splitPath(const QString& path) const
	{
		QStringList toReturn;

		toReturn.push_back("[Global Variables]");

		int start = 0;
		for (int end = start; end < path.length(); ++end) {
			if (path[end] == '.') {
				toReturn.push_back(path.mid(start, end-start));
				start = end+1;
			}
			if (path[end] == ':') {
				toReturn.push_back(path.mid(start, end-start));
				toReturn.push_back("[Methods]");
				start = end+1;
			}
		}
		toReturn.push_back(path.right(path.length() - start));

		return toReturn;
	}


	void LuaCompleter::update()
	{
		m_model->setData(&_luaVmState, LuaTreeItem::CompleterModel);
	}


	// ================================================================================================


	CompletingLuaLineEdit::CompletingLuaLineEdit(sol::state& luaVmState, QWidget* parent)
		: QLineEdit(parent)
		, _completer(nullptr)
	{
		setCompleter(new LuaCompleter(luaVmState, this));
	}

	
	void CompletingLuaLineEdit::setCompleter(LuaCompleter* completer)
	{
		if (_completer)
			QObject::disconnect(_completer, 0, this, 0);

		_completer = completer;

		if (! _completer)
			return;

		_completer->setWidget(this);
		_completer->setCompletionMode(QCompleter::PopupCompletion);
		_completer->setCaseSensitivity(Qt::CaseInsensitive);
		connect(_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
	}


	LuaCompleter* CompletingLuaLineEdit::completer() const
	{
		return _completer;
	}


	void CompletingLuaLineEdit::keyPressEvent(QKeyEvent *e)
	{
		if (_completer && _completer->popup()->isVisible()) {
			// The following keys are forwarded by the completer to the widget
			switch (e->key()) {
				case Qt::Key_Enter:
				case Qt::Key_Return:
				case Qt::Key_Escape:
				case Qt::Key_Tab:
				case Qt::Key_Backtab:
					e->ignore(); 
					return; // let the completer do default behavior
				default:
					break;
			}
		}

		bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+Space
		if (!_completer || !isShortcut) // do not process the shortcut when we have a completer
			QLineEdit::keyPressEvent(e);

		const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
		if (!_completer || (ctrlOrShift && e->text().isEmpty()))
			return;

		QString textUnderCursor = text().left(cursorPosition());
		int pos = int(textUnderCursor.toStdString().find_last_of(" ()[]{}"));
		QString completionPrefix = textUnderCursor.right(textUnderCursor.length() - pos - 1);

		bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
		if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1))
		{
			_completer->popup()->hide();
			return;
		}

		if (completionPrefix != _completer->completionPrefix())
		{
			_completer->setCompletionPrefix(completionPrefix);
			_completer->popup()->setCurrentIndex(_completer->completionModel()->index(0, 0));
		}

		QRect cr = cursorRect();
		cr.setWidth(_completer->popup()->sizeHintForColumn(0) + _completer->popup()->verticalScrollBar()->sizeHint().width());
		_completer->complete(cr);
	}


	void CompletingLuaLineEdit::insertCompletion(QString completitionString)
	{
		if (_completer->widget() != this)
			return;

		QString textUnderCursor = text().left(cursorPosition());
		size_t pos = textUnderCursor.toStdString().find_last_of(" .:()[]{}");
		if (pos == std::string::npos)
			pos = 0;
		else
			pos += 1;

		QString fullText = text();
		fullText.replace(int(pos), textUnderCursor.length() - int(pos), completitionString);
		setText(fullText);
	}

}
}