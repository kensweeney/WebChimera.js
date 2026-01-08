const { contextBridge, ipcRenderer } = require('electron');
const path = require('path');
const EventEmitter = require('events');

// Construct the path to the native module
const wcjsPath = path.join(__dirname, '..', 'build', 'webchimera.js', 'WebChimera.js.node');
const { VlcPlayer } = require(wcjsPath);

// Get the plugin path from the main process
const vlcPluginPath = process.env.VLC_PLUGIN_PATH;

const em = new EventEmitter();

// Create the player instance here in the preload script, now with the plugin path
const player = new VlcPlayer([
    '--verbose=1',
    `--plugin-path=${vlcPluginPath}`,
    '--vout=vmem',
    '--no-video-title-show',
    `--network-caching=300`,
    '--vmem-chroma=RGBA' // Request raw RGBA frames
], em);

// Expose a secure API to the renderer process
contextBridge.exposeInMainWorld('electronAPI', {
    // Expose methods that call the real player instance
    rw: () => {
        if (player.input) {
            player.input.time -= 10000; // Rewind 10 seconds
        }
    },
    ff: () => {
        if (player.input) {
            player.input.time += 10000; // Fast forward 10 seconds
        }
    },
    play: (mrl) => player.play(mrl),
    pause: () => player.pause(),
    stop: () => player.stop(),
    toggleMute: () => player.toggleMute(),
    setVolume: (vol) => {
        if (player.audio) {
            player.audio.volume = parseInt(vol);
        }
    },
    // Expose a way to set callbacks
    on: (eventName, callback) => {
        // Map "onFrameSetup" -> "FrameSetup" for EventEmitter
        let internalName = eventName;
        if (eventName.startsWith('on')) {
            internalName = eventName.substring(2);
        }
        
        // console.log(`Registering listener for ${internalName}`);
        em.on(internalName, callback);

        // Also set the property on the player object as a fallback/primary
        // This triggers the C++ setter
        try {
            player[eventName] = callback;
        } catch (e) {
            console.error(`Failed to set property ${eventName} on player:`, e);
        }
    },

    // We don't need setupCanvas anymore
    onPlayMedia: (callback) => ipcRenderer.on('play-media', (event, ...args) => callback(...args)),
    getVlcPluginPath: () => vlcPluginPath,
});