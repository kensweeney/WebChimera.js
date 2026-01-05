const { ipcRenderer } = require('electron');

const urlInput = document.getElementById('url-input');
const playButton = document.getElementById('play-button');

// When the main process sends the default URL, set it in the input field
ipcRenderer.on('set-url', (event, url) => {
    urlInput.value = url;
});

// Send the URL back to the main process when the button is clicked
playButton.addEventListener('click', () => {
    ipcRenderer.send('submit-url', urlInput.value);
});

// Allow submitting by pressing Enter
urlInput.addEventListener('keyup', (event) => {
    if (event.key === 'Enter') {
        playButton.click();
    }
});