#include <Arduino.h> // Para evitar los errores del parser de Visual Code :/  Pragma rulez

#define PI_2    (PI/2.0)

struct axisData_t {
    char name;
    double lastRealPos; // Último valor recibido en radianes
    double lastPos; // Última posición devuelta

    double movDelta; // % de movimiento aplicado en cada loop sobre el desplazado real (0f, 1f]

    double minRealMovStatic; // Más de esto activa el movimiento
    double maxDeltaStatic; // Menos de esto desactiva el movimiento
    bool isStatic; // Moviendo?

    double headRange; // Radianes de movimiento real de cabeza. Si mueve más allá, siempre 100%. (0, PI/2]
};

// Constantes y seguimiento de estado de cada eje
axisData_t axisDataX, axisDataY, axisDataZ;
unsigned long lastAxisUpdate = 0;

void setAxis() {
    axisDataX.lastRealPos = axisDataY.lastRealPos = axisDataZ.lastRealPos = 0;
    axisDataX.lastPos = axisDataY.lastPos = axisDataZ.lastPos = 0;
    axisDataX.isStatic = axisDataY.isStatic = axisDataZ.isStatic = false;

    axisDataX.name = 'X';
    axisDataX.movDelta = 0.2f;
    axisDataX.minRealMovStatic = 0.01;
    axisDataX.maxDeltaStatic = 0.0015;
    axisDataX.headRange = 0.3;

    axisDataY.name = 'Y';
    axisDataY.movDelta = 0.2f;
    axisDataY.minRealMovStatic = 0.01;
    axisDataY.maxDeltaStatic = 0.0015;
    axisDataY.headRange = 0.25;

    axisDataZ.name = 'Z';
    axisDataZ.movDelta = 0.08f;
    axisDataZ.minRealMovStatic = 0.02;
    axisDataZ.maxDeltaStatic = 0.03;
    axisDataZ.headRange = 0.25;
}

// Transforma un valor global a uno que tenga sentido en movimientos de cabeza, para ser fino y no saltar
double clampAxis(double val, axisData_t *axisData) {
    double ret;
    
    ret = val / axisData->headRange;

    if(ret > PI_2) return PI_2;
    if(ret < -PI_2) return -PI_2;

    return ret;
}

/**
 * @param val           Radianes
 * @param timeDelta     Microsegundos desde última actualización
 * @param axisData      Estado y constantes de cálculo de eje
 * @return              
 */
double processAxis(double val, unsigned long timeDelta, axisData_t *axisData) {
    double delta = val - axisData->lastPos;
    double deltaAbs = delta > 0 ? delta : -delta;

    // Si se mueve mucho, activa
    if(axisData->isStatic && (deltaAbs > axisData->minRealMovStatic)) {
//        Serial1.print(axisData->name);
//        Serial1.print("+ ");
        axisData->isStatic = false;
    }

    // Si está en movimiento actualiza posición
    if(!axisData->isStatic) {
        axisData->lastRealPos = val; // Guardamos pos del último cálculo, porque estamos moviendo

        // Si se mueve, pero poco, hace estático
        if(deltaAbs < axisData->maxDeltaStatic) {
//            Serial1.print(axisData->name);
//            Serial1.print("· ");
            axisData->isStatic = true;
        }
    } else {
        // Estático, así que seguimos con el dumper desde el último valor bueno
        delta = axisData->lastRealPos - axisData->lastPos;
    }

    // Desplaza el % de movDelta
    axisData->lastPos += delta * axisData->movDelta;

    return clampAxis(axisData->lastPos, axisData);
}

void processHead(double x, double y, double z, double *outX, double *outY, double *outZ) {
    unsigned long timeDelta = micros() - lastAxisUpdate;

    *outX = processAxis(asin(x), timeDelta, &axisDataX) / PI_2;
    *outY = processAxis(asin(y), timeDelta, &axisDataY) / PI_2;
    *outZ = processAxis(asin(z), timeDelta, &axisDataZ) / PI_2;

    lastAxisUpdate += timeDelta;
}