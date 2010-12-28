#!/bin/env/ python
#-*- coding=utf8 -*-
import wx
import dialogs_xrc
app = wx.PySimpleApp()
frame = dialogs_xrc.xrcmainframe(parent = None)
frame.Show()
app.MainLoop()
