
#include "scriptingwidget.h"

#include <QKeyEvent>

namespace cts { namespace gui
{
	std::vector<ScriptingWidget*> ScriptingWidget::m_scriptingWidgets;


	ScriptingWidget::ScriptingWidget(sol::state& luaState, QWidget* parent)
		: QWidget(parent)
		, m_luaState(luaState)
		, m_consoleDisplay(nullptr)
		, m_editCommand(nullptr)
		, m_btnExecute(nullptr)
		, m_btnClear(nullptr)
		, m_currentPosition(-1)
	{
		setupGUI();
		m_scriptingWidgets.push_back(this);
		
		m_luaState.set_function("print", &ScriptingWidget::lua_print_callback);
		m_luaState.set_function("debug", &ScriptingWidget::lua_debug_callback);
		connect(this, &ScriptingWidget::newMessage, this, &ScriptingWidget::appendMessage);
	}


	ScriptingWidget::~ScriptingWidget()
	{
		utils::remove_erase(m_scriptingWidgets, this);
	}


	void ScriptingWidget::setupGUI() {
		setWindowTitle(tr("Scripting Console"));

		QVBoxLayout* mainLayout = new QVBoxLayout(this);

		QHBoxLayout* controlsLayout = new QHBoxLayout();
		mainLayout->addLayout(controlsLayout);

		m_consoleDisplay = new QTextEdit(this);
		m_consoleDisplay->setReadOnly(true);
		mainLayout->addWidget(m_consoleDisplay);

		// Use the system's default monospace font at the default size in the log viewer
		QFont monoFont = QFont("Monospace");
		monoFont.setStyleHint(QFont::TypeWriter);
		monoFont.setPointSize(QFont().pointSize() + 1);

		m_consoleDisplay->document()->setDefaultFont(monoFont);
		m_editCommand = new QLineEdit(this);
		m_editCommand->setPlaceholderText(tr("Enter Lua commands here..."));
		m_editCommand->installEventFilter(this);
		controlsLayout->addWidget(m_editCommand);

		m_btnExecute = new QPushButton(tr("&Execute"), this);
		controlsLayout->addWidget(m_btnExecute);

		m_btnClear = new QPushButton(tr("&Clear"), this);
		controlsLayout->addWidget(m_btnClear);


		connect(m_btnClear, &QPushButton::clicked, this, &ScriptingWidget::clearLog);
		connect(m_btnExecute, &QPushButton::clicked, this, &ScriptingWidget::execute);
		connect(m_editCommand, SIGNAL(returnPressed()), this, SLOT(execute()));
	}

	void ScriptingWidget::appendMessage(const QString& message)
	{
		m_consoleDisplay->append(message);
	}

	void ScriptingWidget::clearLog()
	{
		m_consoleDisplay->clear();
	}

	void ScriptingWidget::execute()
	{
		QString command = m_editCommand->text();
		appendMessage(tr("> ") + command);

		const std::string str = command.toStdString();
		try
		{
			m_luaState.script(str);
		}
		catch (std::exception& e)
		{
			appendMessage(tr("Caught exception during execution: %1").arg(e.what()));
		}

		m_history.push_front(command);
		m_currentPosition = -1;
		m_editCommand->clear();
	}


	int ScriptingWidget::lua_print_callback(lua_State* L)
	{
		int nargs = lua_gettop(L);
		lua_getglobal(L, "tostring");
		std::string str;

		for (int i = 1; i <= nargs; i++) {
			const char *s;
			size_t l;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tolstring(L, -1, &l);  /* get result */
			if (s == NULL)
				return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

			if (i > 1)
				str += "\t";
			str += s;
			lua_pop(L, 1);  /* pop result */
		}

		for (auto sw : m_scriptingWidgets)
			sw->newMessage(QString::fromStdString(str));
		return 0;
	}


	int ScriptingWidget::lua_debug_callback(lua_State* L)
	{
		for (;;) {
			char buffer[250];
			//luai_writestringerror("%s", "lua_debug> ");
			if (fgets(buffer, sizeof(buffer), stdin) == 0 || strcmp(buffer, "cont\n") == 0)
				return 0;
			if (luaL_loadbuffer(L, buffer, strlen(buffer), "=(debug command)") || lua_pcall(L, 0, 0, 0))
			{
				std::string str(lua_tostring(L, -1));
				for (auto sw : m_scriptingWidgets)
					sw->newMessage(QString::fromStdString(str));
			}
			lua_settop(L, 0);  /* remove eventual returns */
		}
	}


	bool ScriptingWidget::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj == m_editCommand && event->type() == QEvent::KeyPress) {
			QKeyEvent* e = static_cast<QKeyEvent*>(event);
			if (e->key() == Qt::Key_Up && m_currentPosition < static_cast<int>(m_history.size())-1) {
				++m_currentPosition;
				m_editCommand->setText(m_history[m_currentPosition]);
				return true;
			}
			if (e->key() == Qt::Key_Down && m_currentPosition >= 0) {
				--m_currentPosition;
				if (m_currentPosition >= 0)
					m_editCommand->setText(m_history[m_currentPosition]);
				else
					m_editCommand->setText(tr(""));

				return true;
			}
		}

		return QObject::eventFilter(obj, event);
	}
}
}
