#!/bin/env/ python
#-*- coding=utf8 -*-
import wx
import skyeye_xrc
app = wx.PySimpleApp()
frame = skyeye_xrc.xrcframe(parent = None)
frame.Show()
app.MainLoop()
