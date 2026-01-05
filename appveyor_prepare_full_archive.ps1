# PowerShell script to prepare the full archive for WebChimera.js release
# This is a replacement for appveyor_prepare_full_archive.cmd which fails when downloading the VLC libraries


$VLC_VER = "3.0.21"
$WCJS_FULL_ARCHIVE = "WebChimera.js_x64_VLC-v$VLC_VER"
$BUILD_DIR = ".\build"

# Default to x64 if npm_config_wcjs_arch isn't set
if (-not $env:npm_config_wcjs_arch) {
    $env:npm_config_wcjs_arch = "x64"
}

Write-Host "npm_config_wcjs_arch = '$env:npm_config_wcjs_arch'"

if ($env:npm_config_wcjs_arch -eq "x64") {
    $VLC_ARCHIVE = "vlc-$VLC_VER-win64.7z"
    $VLC_ARCHIVE_URL = "http://get.videolan.org/vlc/$VLC_VER/win64/$VLC_ARCHIVE"
} else {
    $VLC_ARCHIVE = "vlc-$VLC_VER-win32.7z"
    $VLC_ARCHIVE_URL = "http://get.videolan.org/vlc/$VLC_VER/win32/$VLC_ARCHIVE"
}

$FULL_PACKAGE_DIR = "$BUILD_DIR\webchimera.js"
New-Item -ItemType Directory -Path $FULL_PACKAGE_DIR -Force

# Download VLC archive (replaces appveyor DownloadFile)
Write-Host "Downloading $VLC_ARCHIVE_URL to $BUILD_DIR\$VLC_ARCHIVE"
$destination = "$BUILD_DIR\$VLC_ARCHIVE"

# Prefer native curl.exe (follows JS-redirecting pages better), fall back to Invoke-WebRequest
if (Get-Command curl.exe -ErrorAction SilentlyContinue) {
    Write-Host "Using curl.exe to download"
    & curl.exe -L --output $destination $VLC_ARCHIVE_URL
} else {
    Write-Host "Using Invoke-WebRequest to download (with browser user-agent)"
    Invoke-WebRequest -Uri $VLC_ARCHIVE_URL -OutFile $destination -Headers @{ 'User-Agent' = 'Mozilla/5.0 (Windows NT)' } -MaximumRedirection 10
}

# If the downloaded file is small (likely an HTML page), try to parse a real .7z link out of it
try {
    $fi = Get-Item $destination -ErrorAction Stop
} catch {
    Write-Error "Download failed, file not found: $destination"
    exit 1
}

if ($fi.Length -lt 100KB) {
    Write-Host "Downloaded file is small ($($fi.Length) bytes) - inspecting for a real download link"
    $html = Get-Content $destination -Raw -ErrorAction SilentlyContinue
    if (-not $html) {
        Write-Error "Downloaded content is empty and too small. Aborting."
        exit 1
    }

    # Look for a link to a .7z archive in the HTML
    if ($html -match 'href=["'']([^"'']+\.7z)["'']') {
        $link = $matches[1]
        # Resolve relative URLs
        if ($link -notmatch '^https?://') {
            $base = [System.Uri] $VLC_ARCHIVE_URL
            $link = (New-Object System.Uri($base, $link)).AbsoluteUri
        }
        Write-Host "Found candidate .7z link: $link - downloading it"
        if (Get-Command curl.exe -ErrorAction SilentlyContinue) {
            & curl.exe -L --output $destination $link
        } else {
            Invoke-WebRequest -Uri $link -OutFile $destination -Headers @{ 'User-Agent' = 'Mozilla/5.0 (Windows NT)' } -MaximumRedirection 10
        }
        $fi = Get-Item $destination
        if ($fi.Length -lt 100KB) {
            Write-Error "Downloaded .7z candidate is still unexpectedly small ($($fi.Length) bytes). Aborting."
            exit 1
        }
    } else {
        $htmlSave = "$destination.html"
        Set-Content -Path $htmlSave -Value $html -Encoding UTF8
        Write-Error "Download URL returned an HTML page and no .7z link was found. Saved HTML to $htmlSave for inspection."
        exit 1
    }
}

# Extract VLC archive
& 7z x "$destination" -o"$BUILD_DIR"
$VLC_DIR = "$BUILD_DIR\vlc-$VLC_VER"

# Copy the built WebChimera.js.node
Copy-Item "$BUILD_DIR\Release\WebChimera.js.node" $FULL_PACKAGE_DIR

# Create index.js
"module.exports = require('./WebChimera.js.node')" | Out-File "$FULL_PACKAGE_DIR\index.js" -Encoding UTF8

# Copy all VLC plugins
$pluginSrc = Join-Path $VLC_DIR "plugins"
$pluginDest = Join-Path $FULL_PACKAGE_DIR "plugins"
if (Test-Path $pluginSrc) {
    Copy-Item -Path $pluginSrc -Destination $pluginDest -Recurse -Force
} else {
    Write-Host "VLC plugins directory not found at $pluginSrc, skipping."
}


# Copy VLC DLLs
foreach ($dll in @("libvlc.dll","libvlccore.dll")) {
    $src = Join-Path $VLC_DIR $dll
    if (Test-Path $src) {
        Copy-Item $src $FULL_PACKAGE_DIR -Force
    } else {
        Write-Host "$dll not found at $src, skipping."
    }
}

# Create the archive as tar.gz (use 7z to create tar then gzip)
$tarPath = Join-Path $BUILD_DIR "$WCJS_FULL_ARCHIVE.tar"
$gzPath = Join-Path $BUILD_DIR "$WCJS_FULL_ARCHIVE.tar.gz"

Write-Host "Creating tar archive $tarPath"
& 7z a -ttar $tarPath $FULL_PACKAGE_DIR

Write-Host "Gzipping tar archive to $gzPath"
& 7z a -tgzip $gzPath $tarPath

# Remove intermediate tar
if (Test-Path $tarPath) { Remove-Item $tarPath -Force }

Write-Host "Created package: $gzPath"