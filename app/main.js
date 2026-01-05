const { app, BrowserWindow, Menu, dialog, ipcMain, session } = require('electron');
const path = require('path');
const { pathToFileURL } = require('url'); // Import the URL conversion function

// Set a custom environment variable
const vlcBuildDir = path.join(__dirname, '..', 'build', 'webchimera.js');
process.env['VLC_PLUGIN_PATH'] = path.join(vlcBuildDir, 'plugins'); // Correctly point to the plugins subdirectory

// Add the directory containing the VLC DLLs to the process's DLL search path.
// This must be done before the VlcPlayer module is loaded.
if (process.platform === 'x64') {
  try {
    process.addDllDirectory(vlcBuildDir);
  } catch (e) {
    console.error('Failed to add DLL directory:', e);
  }
}

let mainWindow;

function createMainWindow() {
  mainWindow = new BrowserWindow({
    width: 1280,
    height: 720,
    webPreferences: {
      // It's recommended to use a preload script for security
      preload: path.join(__dirname, 'preload.js'),
      // contextIsolation should be true (default)
      contextIsolation: true,
      // nodeIntegration should be false (default)
      nodeIntegration: false,
      // Explicitly disable sandbox for the main window
      sandbox: false
    },
  });

  mainWindow.loadFile('index.html');
  return mainWindow;
}

function createUrlDialog(defaultUrl) {
    const dialogWindow = new BrowserWindow({
        width: 400,
        height: 150,
        parent: mainWindow,
        modal: true,
        show: false,
        webPreferences: {
            nodeIntegration: true, // Allow Node.js APIs in the dialog's renderer
            contextIsolation: false,
        }
    });

    dialogWindow.loadFile('dialog.html');

    dialogWindow.once('ready-to-show', () => {
        dialogWindow.show();
        dialogWindow.webContents.send('set-url', defaultUrl);
    });

    ipcMain.once('submit-url', (event, url) => {
        if (url) {
            mainWindow.webContents.send('play-media', url);
        }
        dialogWindow.close();
    });
}

app.whenReady().then(() => {
  // Apply a Content Security Policy
  session.defaultSession.webRequest.onHeadersReceived((details, callback) => {
    callback({
      responseHeaders: {
        ...details.responseHeaders,
        'Content-Security-Policy': ["script-src 'self'"]
      }
    });
  });

  // You can access the variable here
  console.log(`VLC Plugin Path: ${process.env.VLC_PLUGIN_PATH}`);
  createMainWindow();

  const menuTemplate = [
    {
      label: 'File',
      submenu: [
        {
          label: 'Open Video File...',
          accelerator: 'CmdOrCtrl+O',
          click: () => {
            dialog.showOpenDialog(mainWindow, {
              title: 'Open Video File',
              properties: ['openFile'],
            }).then(result => {
              if (!result.canceled && result.filePaths.length > 0) {
                const fileMrl = pathToFileURL(result.filePaths[0]).href;
                mainWindow.webContents.send('play-media', fileMrl);
              }
            });
          },
        },
        { type: 'separator' },
        {
          label: 'Exit',
          role: 'quit',
        },
      ],
    },
    {
      label: 'Streams',
      submenu: [
        {
          label: 'Open UDP Stream...',
          click: () => createUrlDialog('udp://@:1234'),
        },
        {
          label: 'Open HTTP Stream...',
          click: () => createUrlDialog('http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4'),
        },
        {
          label: 'Open RTSP Stream...',
          click: () => createUrlDialog('rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov'),
        },
        {
          label: 'Open Custom URL...',
          click: () => createUrlDialog(''),
        }
      ],
    },
    {
      label: 'Developer',
      submenu: [
        { role: 'toggleDevTools' }
      ]
    }
  ];

  const menu = Menu.buildFromTemplate(menuTemplate);
  Menu.setApplicationMenu(menu);

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createMainWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});