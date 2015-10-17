/*
 * mainwindow.h
 */

#ifndef UI_MAINWINDOW_H_
#define UI_MAINWINDOW_H_

#include <memory>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

class MainWindow : public Fl_Window
{
public:
	MainWindow();
	virtual ~MainWindow();

private:
	std::unique_ptr<Fl_Box> m_box;
};

#endif /* UI_MAINWINDOW_H_ */
