#pragma once
const int BASE_SLEEP_TIME = 10;
const int MAX_ADJUST_TIME = 5;
/*
 * 以BASE_SLEEP_TIME(ms)为一个基准时间，假定所有视频的渲染帧数都不会超过100FPS
 * 以MAX_ADJUST_TIME(ms)为一个调整周期，假定每一帧的调整都不会超过原定时间周期的MAX_ADJUST_TIME(ms)范围波动
 */