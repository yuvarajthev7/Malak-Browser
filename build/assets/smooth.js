// Malak Browser Universal Physics Engine
// Supports: Window Scroll, YouTube (ytd-app), Sidebars, and Chat boxes.

const SCROLL_SPEED = 300;
const FRICTION = 1; // Lower = snappier, Higher = floatier

// Helper: Find the actual scrollable parent of the element under the mouse
function getScrollParent(node) {
    if (!node) return window;

    // Walk up the tree
    while (node && node !== document.body && node !== document.documentElement) {
        const style = window.getComputedStyle(node);
        const overflow = style.getPropertyValue('overflow-y');

        // If this element has scrollbars and content that is too tall...
        const canScroll = (overflow === 'auto' || overflow === 'scroll')
                          && (node.scrollHeight > node.clientHeight);

        if (canScroll) return node; // Found it! (e.g., ytd-app)

        node = node.parentElement;
    }
    // If we reached the top, the scroller is the main window
    return window;
}

// Helper: The animation loop for a specific element
function animateScroll(el) {
    if (!el._isScrolling) return;

    // Calculate next step
    const current = (el === window) ? window.scrollY : el.scrollTop;
    const diff = el._targetY - current;
    const step = diff / FRICTION;

    // Apply movement
    const nextPos = current + step;

    if (el === window) window.scrollTo(0, nextPos);
    else el.scrollTop = nextPos;

    // Stop if close enough
    if (Math.abs(diff) < 0.5) {
        if (el === window) window.scrollTo(0, el._targetY);
        else el.scrollTop = el._targetY;

        el._isScrolling = false; // Stop loop
    } else {
        requestAnimationFrame(() => animateScroll(el)); // Continue loop
    }
}

window.addEventListener('wheel', function(e) {
    // 1. Ignore text inputs (let user scroll code/text)
    if (e.target.tagName === 'TEXTAREA' || e.target.isContentEditable) return;

    // 2. Find WHO we should scroll
    const scroller = getScrollParent(e.target);

    // 3. Prevent native jank
    e.preventDefault();

    // 4. Initialize tracker variables if missing
    // We attach these "hidden" variables directly to the element
    if (typeof scroller._targetY === 'undefined') {
        scroller._targetY = (scroller === window) ? window.scrollY : scroller.scrollTop;
    }

    // 5. Update Target
    // If we just started scrolling, sync target with current position first
    if (!scroller._isScrolling) {
        scroller._targetY = (scroller === window) ? window.scrollY : scroller.scrollTop;
    }

    let direction = e.deltaY > 0 ? 1 : -1;
    scroller._targetY += direction * SCROLL_SPEED;

    // 6. Clamp Limits (Prevent scrolling past top/bottom)
    const maxScroll = (scroller === window)
        ? document.body.scrollHeight - window.innerHeight
        : scroller.scrollHeight - scroller.clientHeight;

    scroller._targetY = Math.max(0, Math.min(scroller._targetY, maxScroll));

    // 7. Start Animation if not running
    if (!scroller._isScrolling) {
        scroller._isScrolling = true;
        requestAnimationFrame(() => animateScroll(scroller));
    }

}, { passive: false });
