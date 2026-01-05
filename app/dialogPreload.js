const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('dialogAPI', {
  onSetUrl: (callback) => ipcRenderer.on('set-url', (event, ...args) => callback(...args)),
  submitUrl: (url) => ipcRenderer.send('submit-url', url),
});