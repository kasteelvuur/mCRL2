// Author: Ruud Koolen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MCRL2_UTILITIES_QT_TOOL_H
#define MCRL2_UTILITIES_QT_TOOL_H

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QString>
#include <QUrl>
#include <QtGlobal>
#include <string>
#include "toolset_version.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace mcrl2
{
namespace utilities
{
namespace qt
{

class QtToolBase : public QObject
{
  Q_OBJECT

  private:
    QString m_name;
    QString m_author;
    QString m_description;
    QString m_manualUrl;
    QMainWindow *m_window;

  protected:
    bool show_main_window(QMainWindow *window)
    {
      m_window = window;
      QAction *actionContents = new QAction(window);
      actionContents->setText("&Contents");
      actionContents->setShortcut(QString("F1"));
      connect(actionContents, SIGNAL(triggered()), this, SLOT(showContents()));

      QAction *actionAbout = new QAction(window);
      actionAbout->setText("&About");
      connect(actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

      QMenu *menuHelp = new QMenu(window->menuBar());
      menuHelp->setTitle("&Help");
      menuHelp->addAction(actionContents);
      menuHelp->addSeparator();
      menuHelp->addAction(actionAbout);
      window->menuBar()->addAction(menuHelp->menuAction());

      window->show();
      return QApplication::instance()->exec() == 0;
    }

  protected slots:
    void showContents()
    {
      QDesktopServices::openUrl(QUrl(m_manualUrl));
    }

    void showAbout()
    {
      QString version = QString(mcrl2::utilities::get_toolset_version().c_str());
      QString description = m_description;
      description.replace("\n", "<br>");
      QString message;
      message += "<h1>" + m_name + "</h1>";
      message += "<p>" + description + "</p>";
      message += "<p>Written by " + m_author + "</p>";
      message += "<\br>";
      message += "<p>Version: " + version + "</p>";
      QMessageBox::about(m_window, QString("About ") + m_name, message);
    }

  public:
    QtToolBase(QString name, QString author, QString description, QString manualUrl):
      m_name(name),
      m_author(author),
      m_description(description),
      m_manualUrl(manualUrl)
    {}
};

template <typename Tool>
class qt_tool: public Tool, public QtToolBase
{
  private:
#if defined(_WIN32) 
	std::streambuf	*_cinbuf;
	std::streambuf	*_coutbuf;
	std::streambuf	*_cerrbuf;
	std::ifstream	_console_cin;
	std::ofstream	_console_cout;
	std::ofstream	_console_cerr;
#endif // _WIN32
    int m_argc;
    char** m_argv;
  protected:
    QApplication* m_application;
  public:
    qt_tool(const std::string& name,
            const std::string& author,
            const std::string& what_is,
            const std::string& tool_description,
            const std::string& about_description,
            const std::string& manual_url,
            std::string known_issues = ""
           )
      : Tool(name, author, what_is, tool_description, known_issues),
        QtToolBase(QString::fromStdString(name), QString::fromStdString(author), QString::fromStdString(about_description), QString::fromStdString(manual_url)),
        m_application(NULL)
    {
#if defined(_WIN32)
       BOOL success = AttachConsole(ATTACH_PARENT_PROCESS);
       if (success)
       {
	     _cinbuf = std::cin.rdbuf();
	     _console_cin.open("CONIN$");
	     std::cin.rdbuf(_console_cin.rdbuf());
	     _coutbuf = std::cout.rdbuf();
	     _console_cout.open("CONOUT$");
	     std::cout.rdbuf(_console_cout.rdbuf());
	     _cerrbuf = std::cerr.rdbuf();
	     _console_cerr.open("CONOUT$");
	     std::cerr.rdbuf(_console_cerr.rdbuf());
       }
#endif
    }

    virtual bool pre_run()
    {
#if defined(_WIN32)
	  _console_cout.close();
	  std::cout.rdbuf(_coutbuf);
	  _console_cin.close();
	  std::cin.rdbuf(_cinbuf);
	  _console_cerr.close();
	  std::cerr.rdbuf(_cerrbuf);
	  FreeConsole();
#endif
      m_application = new QApplication(m_argc, m_argv);
      qsrand(QDateTime::currentDateTime().toTime_t());
      return true;
    }

    int execute(int& argc, char** argv)
    {
      m_argc = argc;
      m_argv = argv;
      return static_cast<Tool *>(this)->execute(argc, argv);
    }

    virtual ~qt_tool()
    {
      delete m_application;
    }
};

} // namespace qt
} // namespace utilities
} // namespace mcrl2

#endif // MCRL2_UTILITIES_QT_TOOL_H
