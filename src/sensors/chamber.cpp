#if MINIMOTE

#include <SparkFun_TMP117.h>

static TMP117 temp;
static bool ready = false;

bool chamber_init() {
    ready = temp.begin();
    return ready;
}

float chamber_read() {
    if (!ready || !temp.dataReady()) {
        return -1.f;
    }
    return temp.readTempC();
}

#endif // MINIMOTE
