import { app, BrowserWindow, ipcMain, webContents } from 'electron'
import config from './config'
import minimist from 'minimist'
import mkpath from 'mkpath'

if (require('electron-squirrel-startup')) {
  app.quit()
}

mkpath.sync(config.userPluginPath)

app.commandLine.appendSwitch('--enable-experimental-web-platform-features')

app.on('window-all-closed', () => {
  for (const wc of webContents.getAllWebContents()) {
    wc.closeDevTools()
  }
  app.quit()
})

function newWindow (profile = 'default') {
  const options = {
    width: 1200,
    height: 600,
    show: false,
    titleBarStyle: 'hidden-inset',
    vibrancy: 'ultra-dark',
  }
  if (process.platform === 'linux') {
    options.icon = '/usr/share/icons/hicolor/256x256/apps/deplug.png'
  }
  const mainWindow = new BrowserWindow(options)
  mainWindow.loadURL(`file://${__dirname}/index.htm`)
  const contents = mainWindow.webContents
  contents.on('crashed', () => mainWindow.reload())
  contents.on('unresponsive', () => mainWindow.reload())
  contents.on('dom-ready', () => {
    const argv = JSON.stringify(minimist(process.argv.slice(2)))
    const profileId = JSON.stringify(profile)
    const script = `require("./window.main.js")(${profileId}, ${argv})`
    contents.executeJavaScript(script)
  })
}

app.on('ready', () => {
  newWindow()
})

ipcMain.on('new-window', (event, profile) => {
  newWindow(profile)
})

ipcMain.on('window-deplug-loaded', (event, id) => {
  BrowserWindow.fromId(id).show()
})
