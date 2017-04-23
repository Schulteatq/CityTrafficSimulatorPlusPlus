#ifndef CTS_GUI_COMPLETING_LUA_LINE_EDIT_H__
#define CTS_GUI_COMPLETING_LUA_LINE_EDIT_H__

#include <cts-gui/config.h>

#include <sol.hpp>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>

namespace cts { namespace gui {

	class LuaCompleter : public QCompleter {
	public:
		LuaCompleter(sol::state& luaVmState, QWidget* parent);

		virtual ~LuaCompleter() = default;

		virtual QStringList splitPath(const QString& path) const override;

	private:
		sol::state& _luaVmState;
	};


	class CompletingLuaLineEdit : public QLineEdit {
		Q_OBJECT;

	public:
		CompletingLuaLineEdit(sol::state& luaVmState, QWidget* parent);

		virtual ~CompletingLuaLineEdit() = default;

		void setCompleter(LuaCompleter* completer);
		LuaCompleter* completer() const;

	protected:
		void keyPressEvent(QKeyEvent *e);

	private slots:
		void insertCompletion(QString completitionString);

	private:
		LuaCompleter* _completer;

	};

}
}

#endif
