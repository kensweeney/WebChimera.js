// Wait for the DOM to be fully loaded before running the script
document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById("video-canvas");
    console.log("Canvas element found:", canvas);

    // Get the 2D rendering context instead of using WebGL
    const ctx = canvas.getContext('2d');
    console.log("2D rendering context obtained:", ctx);

    let imageData = null;

    // Control buttons
    const muteBtn = document.getElementById('mute-btn');
    const pauseBtn = document.getElementById('pause-btn');
    const stopBtn = document.getElementById('stop-btn');
    const playBtn = document.getElementById('play-btn');

    // Set callbacks using the new 'on' method
    window.electronAPI.on('onFrameSetup', (width, height, pixelFormat, frame) => {
        console.log(`>>> onFrameSetup: Resizing canvas to ${width}x${height}. Pixel Format: ${pixelFormat}`);
        canvas.width = width;
        canvas.height = height;
        // Create an ImageData object to hold the pixel data
        imageData = ctx.createImageData(width, height);
    });

    window.electronAPI.on('onFrameReady', (frame) => {
        if (!frame || !imageData) {
            return;
        }
        
        // Copy the received frame data into the ImageData object
        imageData.data.set(frame);

        // Draw the ImageData onto the canvas
        ctx.putImageData(imageData, 0, 0);
    });

    window.electronAPI.on('onLogMessage', (level, message, format) => {
        if (level >= 3) {
            console.warn(`[VLC Log ${level}]: ${message}`);
        }
    });

    // Handle player state changes
    window.electronAPI.on('onPlaying', () => {
        console.log('Playback event: Playing');
    });
    window.electronAPI.on('onPaused', () => {
        console.log('Playback event: Paused');
    });
    window.electronAPI.on('onStopped', () => {
        console.log('Playback event: Stopped');
    });
    window.electronAPI.on('onEndReached', () => {
        console.log('Playback event: End Reached');
    });

    // Listen for 'play-media' commands from the main process
    window.electronAPI.onPlayMedia((mrl) => {
        console.log('Received play-media command:', mrl);
        window.electronAPI.play(mrl);
    });

    muteBtn.addEventListener('click', () => {
        window.electronAPI.toggleMute();
    });

    // Add event listeners for control buttons
    pauseBtn.addEventListener('click', () => {
        window.electronAPI.pause();
    });

    stopBtn.addEventListener('click', () => {
        window.electronAPI.stop();
    });
    
    playBtn.addEventListener('click', () => {
        window.electronAPI.play();
     });

    // Get the VLC plugin path from the main process
    const vlcPath = window.electronAPI.getVlcPluginPath();
    console.log(`Plugin path from renderer: ${vlcPath}`);
});