// Malak Browser: Logic-Based Ad Skipper v6
console.log("Malak Browser: Safe Skipper Active");

function handleAds() {
    const video = document.querySelector('video');
    if (!video) return;

    // --- SAFETY CHECK 1: Duration ---
    // If video is longer than 5 minutes (300s), it is DEFINITELY not an ad.
    // Do not touch it.
    if (video.duration > 300) {
        if (video.playbackRate > 2.0) video.playbackRate = 1.0; // Restore speed if we messed up
        return;
    }

    // --- DETECTION ---
    const adOverlay = document.querySelector('.ad-showing');
    const skipBtn = document.querySelector('.ytp-ad-skip-button, .ytp-ad-skip-button-modern, .videoAdUiSkipButton');

    // It's only an ad if we have the overlay class OR a skip button
    if (adOverlay || skipBtn) {

        // --- AD ACTION ---
        video.muted = true;
        video.playbackRate = 16.0;

        // Skip to end safely (leave 0.1s buffer)
        if (isFinite(video.duration)) {
            video.currentTime = video.duration - 0.1;
        }

        if (skipBtn) skipBtn.click();

    } else {
        // --- RESET ACTION ---
        // If no ad is detected, force speed back to normal
        if (video.playbackRate > 1.0) {
            video.playbackRate = 1.0;
            // video.muted = false; // Optional: let user handle mute
        }
    }
}

// Run every 100ms
setInterval(handleAds, 100);
