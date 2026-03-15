#include "interpolator.h"



const int16_t arcsin_lut[LUT_SIZE + 1] PROGMEM =
{
0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,
80,85,90,95,100,105,110,115,120,125,130,135,140,145,150,155,
160,165,170,175,180,185,190,195,200,205,210,215,220,225,230,235,
240,245,250,255,260,265,270,275,280,285,290,295,300,305,310,315,
320,325,330,335,340,345,350,355,360,365,370,375,380,385,390,395,
400,405,410,415,420,425,430,435,440,445,450,455,460,465,470,475,
480,485,490,495,500,505,510,515,520,525,530,535,540,545,550,555,
560,565,570,575,580,585,590,595,600,605,610,615,620,625,630,635,
640,645,650,655,660,665,670,675,680,685,690,695,700,705,710,715,
720,725,730,735,740,745,750,755,760,765,770,775,780,785,790,795,
800,805,810,815,820,825,830,835,840,845,850,855,860,865,870,875,
880,885,890,895,900,905,910,915,920,925,930,935,940,945,950,955,
960,965,970,975,980,985,990,995,1000,1005,1010,1015,1020,1025,1030,1035,
1040,1045,1050,1055,1060,1065,1070,1075,1080,1085,1090,1095,1100,1105,1110,1115,
1120,1125,1130,1135,1140,1145,1150,1155,1160,1165,1170,1175,1180,1185,1190,1195,
1200,1205,1210,1215,1220,1225,1230,1235,1240,1245,1250,1255,1260,1265,1270,1275,
1280
};

const int16_t sin_lut[PHASE_BINS] PROGMEM =
{
     0,  400,  784, 1137, 1448, 1703, 1892, 2009,
  2048, 2009, 1892, 1703, 1448, 1137,  784,  400,
     0, -400, -784,-1137,-1448,-1703,-1892,-2009,
 -2048,-2009,-1892,-1703,-1448,-1137, -784, -400
};


/* -------- Testdaten -------- */

static const int16_t testdata[] =
{
3386, 3386, 3390, 3387, 3387, 3386, 3386, 3385, 3385, 3384, 3383, 3385, 3387, 3386, 3384, 3386, 3385, 3385, 3382, 3386, 3385, 3386, 3385, 3386, 3385, 3385, 3383, 3389, 3386, 3386, 3387, 3386, 3386, 3386, 3386, 3385, 3386, 3387, 3389, 3385, 3386, 3385, 3384, 3387, 3385, 3385, 3384, 3385, 3383, 3388, 3387, 3389, 3383, 3387, 3387, 3387, 3384, 3388, 3387, 3385, 3386, 3386, 3387, 3385,
3387, 3385, 3388, 3388, 3386, 3390, 3388, 3390, 3389, 3389, 3385, 3389, 3390, 3390, 3389, 3392, 3389, 3391, 3392, 3391, 3392, 3393, 3393, 3397, 3392, 3393, 3398, 3393, 3393, 3396, 3396, 3396, 3396, 3395, 3398, 3395, 3397, 3399, 3397, 3398, 3397, 3399, 3403, 3400, 3402, 3398, 3400, 3401, 3404, 3404, 3405, 3403, 3403, 3403, 3405, 3407, 3406, 3406, 3405, 3406, 3407, 3407, 3408, 3411,
3410, 3407, 3411, 3410, 3412, 3414, 3413, 3413, 3414, 3416, 3416, 3415, 3418, 3418, 3420, 3420, 3419, 3421, 3419, 3420, 3421, 3421, 3422, 3423, 3425, 3424, 3425, 3425, 3430, 3427, 3427, 3424, 3429, 3429, 3434, 3430, 3428, 3431, 3431, 3431, 3433, 3432, 3432, 3437, 3436, 3442, 3437, 3437, 3440, 3439, 3436, 3437, 3439, 3439, 3442, 3441, 3441, 3444, 3446, 3442, 3446, 3447, 3450, 3449,
3447, 3449, 3450, 3448, 3451, 3452, 3453, 3454, 3453, 3457, 3455, 3454, 3457, 3457, 3459, 3461, 3460, 3461, 3459, 3462, 3462, 3464, 3465, 3465, 3466, 3467, 3466, 3468, 3469, 3469, 3473, 3468, 3471, 3472, 3474, 3473, 3473, 3473, 3476, 3479, 3479, 3480, 3481, 3485, 3486, 3484, 3485, 3488, 3488, 3488, 3490, 3488, 3490, 3489, 3490, 3494, 3495, 3493, 3493, 3495, 3494, 3495, 3497, 3497,
3501, 3499, 3499, 3505, 3500, 3500, 3506, 3506, 3506, 3503, 3505, 3506, 3509, 3510, 3507, 3509, 3509, 3514, 3512, 3512, 3516, 3514, 3517, 3515, 3517, 3514, 3518, 3525, 3515, 3521, 3516, 3521, 3522, 3522, 3525, 3523, 3523, 3526, 3523, 3532, 3529, 3530, 3529, 3530, 3532, 3531, 3530, 3533, 3533, 3538, 3536, 3536, 3539, 3532, 3536, 3539, 3539, 3539, 3538, 3536, 3538, 3539, 3541, 3544
};


int testMain(void)
{
    sin_tracker_t tracker;

    /* grobe Startwerte */
    sintracker_init(&tracker, 3400, 200);

    size_t n =
        sizeof(testdata) /
        sizeof(testdata[0]);

    for(size_t i=0;i<n;i++)
    {
        int16_t sample = testdata[i];

        sintracker_process(&tracker, sample);

        printf(
            "%5zu  adc=%d  phase16=%u  offset=%d  amp=%d  periods=%lu\n",
            i,
            sample,
            tracker.phase16,
            tracker.offset,
            tracker.amplitude,
            tracker.period_counter
        );
    }

    return 0;
}


void sintracker_prefill(sin_tracker_t *s)
{
    for(uint8_t i=0;i<PHASE_BINS;i++)
    {
        int16_t sinv = pgm_read_word(&sin_lut[i]);

        int16_t model =
            s->offset +
            ((int32_t)s->amplitude * sinv) / SIN_SCALE;

        s->bin[i].sum = model;
        s->bin[i].count = 1;
    }
}

void sintracker_init(
        sin_tracker_t *s,
        int16_t offset,
        int16_t amplitude)
{
    s->offset = offset;
    s->amplitude = amplitude;

    s->current_bin = 0;
    s->halfwave = 0;
    s->period_counter = 0;

    sintracker_prefill(s);
}

void sintracker_recalculate(sin_tracker_t *s)
{
    int16_t min = 32767;
    int16_t max = -32768;

    int32_t mean = 0;

    for(uint8_t i=0;i<PHASE_BINS;i++)
    {
        int16_t v =
            s->bin[i].sum /
            s->bin[i].count;

        if(v < min) min = v;
        if(v > max) max = v;

        mean += v;
    }

    s->offset = mean / PHASE_BINS;
    s->amplitude = (max - min) / 2;

    if(s->amplitude < 1)
        s->amplitude = 1;
}


uint8_t sintracker_estimate_bin(
        sin_tracker_t *s,
        int16_t sample)
{
    int16_t centered = sample - s->offset;

    int32_t norm =
        ((int32_t)centered * SIN_SCALE) /
        s->amplitude;

    if(norm > SIN_SCALE) norm = SIN_SCALE;
    if(norm < -SIN_SCALE) norm = -SIN_SCALE;

    uint8_t best = 0;
    int32_t best_err = 0x7FFFFFFF;

    for(uint8_t i=0;i<PHASE_BINS;i++)
    {
        int16_t sinv = pgm_read_word(&sin_lut[i]);

        int32_t err = sinv - norm;

        if(err < 0) err = -err;

        if(err < best_err)
        {
            best_err = err;
            best = i;
        }
    }

    return best;
}

void sintracker_process(
        sin_tracker_t *s,
        int16_t adc)
{
    int16_t centered = adc - s->offset;

    /* Nulldurchgang mit Hysterese */

    if(centered > HYSTERESIS && s->halfwave==0)
    {
        s->halfwave = 1;

        sintracker_recalculate(s);

        sintracker_prefill(s);

        s->period_counter++;
    }
    else
    if(centered < -HYSTERESIS && s->halfwave==1)
    {
        s->halfwave = 0;
    }

    /* Phase bestimmen */

    uint8_t bin =
        sintracker_estimate_bin(s, adc);

    s->current_bin = bin;

    /* Bin mitteln */

    s->bin[bin].sum += adc;
    s->bin[bin].count++;

    /* 16-fach Phase */

    s->phase16 =
        (bin * 16) / PHASE_BINS;
}

