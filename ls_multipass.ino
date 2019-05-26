static const int MULTIPASS_CLOCK_DIVISOR = 12;
static const int MULTIPASS_CLOCK_OFF = 8;
static const int MULTIPASS_SCALE_COUNT = 8;
static int multipass_clock_counter = 0;
static int scale[MULTIPASS_SCALE_COUNT] = {0, 3, 4, 6, 7, 9, 10, 12};

static const int MAXX = 16;
static const int MAXY = 8;
static const int NOTES = 4;
int gol_notes[NOTES] = { 0 };
static int gol_states[MAXX][MAXY][2];
int gol_gen;

int gol_neighbours(int x, int y, int gen) {
    return 
        (gol_states[(x+MAXX-1)%MAXX][(y+MAXY-1)%MAXY][gen] > 0) +
        (gol_states[(x+0)%MAXX][(y+MAXY-1)%MAXY][gen] > 0) +
        (gol_states[(x+1)%MAXX][(y+MAXY-1)%MAXY][gen] > 0) +
        (gol_states[(x+MAXX-1)%MAXX][(y+0)%MAXY][gen] > 0) +
        (gol_states[(x+1)%MAXX][(y+0)%MAXY][gen] > 0) +
        (gol_states[(x+MAXX-1)%MAXX][(y+1)%MAXY][gen] > 0) +
        (gol_states[(x+0)%MAXX][(y+1)%MAXY][gen] > 0) +
        (gol_states[(x+1)%MAXX][(y+1)%MAXY][gen] > 0);
}

static void gol_next_gen(void) {
    int nextgen = gol_gen ? 0 : 1;
    for (int x = 0; x < MAXX; x++)
        for (int y = 0; y < MAXY; y++) {
            int n = gol_neighbours(x, y, gol_gen);
            if (gol_states[x][y][gol_gen] && (n == 2 || n == 3)) {
                gol_states[x][y][nextgen] = 1;
            } else if (!gol_states[x][y][gol_gen] && (n == 3)) {
                gol_states[x][y][nextgen] = 2;
            } else {
                gol_states[x][y][nextgen] = 0;
            }
        }
    gol_gen = nextgen;
}

static void gol_update_leds(void) {
    for (int x = 0; x < MAXX; x++)
        for (int y = 0; y < MAXY; y++)
            if (gol_states[x][y][gol_gen])
                setLed(x + 1, y, gol_states[x][y][gol_gen] == 1 ? COLOR_RED : COLOR_GREEN, cellOn, LED_LAYER_CUSTOM2);
            else
                clearLed(x + 1, y, LED_LAYER_CUSTOM2);
}

static void gol_send_notes(void) {
    int count = 0;
    for (int x = 0; x < MAXX; x++)
        for (int y = 0; y < MAXY; y++)
            count += gol_states[x][y][gol_gen] * x * y;
            
    gol_notes[3] = gol_notes[2];
    gol_notes[2] = gol_notes[1];
    gol_notes[1] = gol_notes[0];
    gol_notes[0] = scale[count % MULTIPASS_SCALE_COUNT] + 60;
    
    for (int i = 0; i < NOTES; i++)
        midiSendNoteOn(LEFT, gol_notes[i], 80, i + 1);
}

static void gol_clear_notes(void) {
    for (int i = 0; i < NOTES; i++)
        midiSendNoteOff(LEFT, gol_notes[i], 1);
}

void multipass_clock(void) {
    if (!userFirmwareActive) return;
    
    if (++multipass_clock_counter < MULTIPASS_CLOCK_DIVISOR) {
        if (multipass_clock_counter == MULTIPASS_CLOCK_OFF) gol_clear_notes();
        return;
    }
    multipass_clock_counter = 0;
    gol_next_gen();
    gol_update_leds();
    gol_send_notes();
}

void multipass_press(int col, int row) {
    if (!userFirmwareActive) return;
    
    DEBUGPRINT((-1,"multipass_press col = ")); DEBUGPRINT((-1,(int)col));
    DEBUGPRINT((-1," row = ")); DEBUGPRINT((-1,(int)row));
    DEBUGPRINT((-1,"\n"));

    if (col == 0) return;
    gol_states[col-1][row][gol_gen] = gol_states[col-1][row][gol_gen] ? 0 : 2;
}

void multipass_release(int col, int row) {
    if (!userFirmwareActive) return;
    
    DEBUGPRINT((-1,"multipass_press col = ")); DEBUGPRINT((-1,(int)col));
    DEBUGPRINT((-1," row = ")); DEBUGPRINT((-1,(int)row));
    DEBUGPRINT((-1,"\n"));

    if (col == 0) return;
}
