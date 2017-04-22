#ifndef SCRIPTINGWIDGET_H__
#define SCRIPTINGWIDGET_H__

#include <cts-core/base/log.h>
#include <cts-core/base/signal.h>
#include <cts-gui/config.h>

#include <deque>

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>

#include <sol.hpp>

namespace cts { namespace gui
{
	/// Qt widget providing a console-like interface to the Lua VM of CampvisApplication.
	class CTS_GUI_API ScriptingWidget : public QWidget {
		Q_OBJECT;

	public:
		/// Creates a new ScriptingWidget
		/// \param  luaState	Lua state object to use.
		/// \param  parent		Parent Qt widget.
		explicit ScriptingWidget(sol::state& luaState, QWidget* parent = nullptr);

		/// Default dtor
		~ScriptingWidget();


	protected:
		/// Setup the widget
		void setupGUI();

		bool eventFilter(QObject* obj, QEvent* event);


	protected slots:
		/// Adds a new message to the text display
		void appendMessage(const QString& message);

		/// Delete all messages from the text display
		void clearLog();

		void execute();

		static int lua_print_callback(lua_State* L);
		static int lua_debug_callback(lua_State* L);

	signals:
		void newMessage(QString message);

	private:
		sol::state& m_luaState;			///< The Lua state to use
		QTextEdit* m_consoleDisplay;	///< Text edit to hold the console output
		QLineEdit* m_editCommand;		///< Text field to enter Lua commands
		QPushButton* m_btnExecute;		///< Button to execute command
		QPushButton* m_btnClear;		///< Button to clear the console output

		std::deque<QString> m_history;	///< History of executed commands
		int m_currentPosition;			///< Current position in command history

		static std::vector<ScriptingWidget*> m_scriptingWidgets;
	};
}
}

#endif // SCRIPTINGWIDGET_H__
