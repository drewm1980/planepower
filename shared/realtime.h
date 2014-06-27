// NO INCLUDE GUARD.  These functions do NOT support nesting,
// i.e. if you ~do nest them, the last part of the outer enable/disable
// region will NOT be realtime!

// Set process piority to realtime and pin cpu clock speed to maximum for first core
void realtime_enable();
void realtime_disable();
void realtime_print_priority();

