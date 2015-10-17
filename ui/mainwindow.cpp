/*
 * mainwindow.cpp
 *
 *  Created on: 17 окт. 2015 г.
 *      Author: todin
 */

#include "mainwindow.h"

MainWindow::MainWindow() : Fl_Window(800, 600)
{
	m_box = std::make_unique<Fl_Box>(10, 10, 780, 580, "Goldmine-Quik Gateway");
	m_box->box(FL_UP_BOX);
	m_box->labelfont(FL_BOLD);
	this->end();
}

MainWindow::~MainWindow()
{
}

