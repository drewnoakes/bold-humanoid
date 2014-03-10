/**
 * @author Drew Noakes http://drewnoakes.com
 */

export interface AgentFrame
{
    /** [x,y,z] */
    ball: number[];

    /** [[x,y,z],...] */
    goals: number[][];

    /** [[x1,y1,z1,x2,y2,z2],...] */
    lines: number[][];

    /** [[x,y],...] */
    visibleFieldPoly: number[][];

    /** [[x1,y1,x2,y2],...] */
    occlusionRays: number[][];
}

export interface Ambulator
{
    /** [x,y,turn] */
    target: number[];
    /** [x,y,turn] */
    current: number[];
    /** [x,y,turn] */
    delta: number[];
    running: boolean;
    phase: number;
    hipPitch: number;
    bodySwingY: number;
    bodySwingZ: number;
}

export interface JointControlData
{
    v: number;
    p: number;
    i: number;
    d: number;
}

export interface BodyControl
{
    cycle: number;
    joints: JointControlData[];
}

export interface Body
{
    'motion-cycle': number;
    angles: number[];
    /** Differences between current and control. */
    errors: number[];
}

export interface CameraFrame
{
    /** [x,y] */
    ball: number[];
    /** [[x,y],...] */
    goals: number[];
    /** [[x1,y1,x2,y2],...] */
    lines: number[];
    totalPixelCount: number;
    processedPixelCount: number;
}

export interface Debug
{
    gameControllerMessages: number;
    ignoredMessages: number;
    sentTeamMessage: number;
    receivedTeamMessages: number;
    /** [r,g,b] as bytes */
    eyeColor: number[];
    /** [r,g,b] as bytes */
    foreheadColor: number[];
    /** [b,b,b] as booleans */
    led: number[];
}

export interface PlayerData
{
    penalty: string;
    penaltySecondsRemaining?: number;
}

export interface TeamData
{
    num: number;
    score: number;
    players: PlayerData[];
}

export interface Game
{
    playMode: string;
    playerPerTeam: number;
    isFirstHalf: boolean;
    nextKickOffTeamNumber: boolean;
    isPenaltyShootOut: boolean;
    isOvertime: boolean;
    lastDropInTeamNumber: number;
    secSinceDropIn: number;
    secondsRemaining: number;
    team1: TeamData;
    team2: TeamData;
}

export interface JointSensorData
{
    id: number;
    val: number;
    rpm: number;
    load: number;
    temp: number;
    volts: number;
}

export interface Hardware
{
    cycle: number;
    /** [x,y,z] */
    acc: number[];
    /** [x,y,z] */
    gyro: number[];
    /** [r,g,b] */
    eye: number[];
    /** [r,g,b] */
    forehead: number[];
    led2: boolean;
    led3: boolean;
    led4: boolean;
    volts: number;
    rxBytes: number;
    txBytes: number;
    joints: JointSensorData[];
}

export interface LabelCount
{
    labels: {name:string; id:number; count:number}[];
}

export interface BodySectionTaskData
{
    module: string;
    priority: number; // TODO is this an enum?
    committed: boolean;
}

export interface MotionTask
{
    head: BodySectionTaskData;
    arms: BodySectionTaskData;
    legs: BodySectionTaskData;
}

export interface Odometry
{
    /** [x,y,z] */
    translation: number[];
}

export interface TeammateData
{
    robotId: number;
    lastUpdate: number;
    state: number;
    role: number;
    x: number;
    y: number;
    theta: number;
    ballX: number;
    ballY: number;
}

export interface OpenTeam
{
    teammates: TeammateData[];
}

export interface OptionData
{
    id: string;
    type: string;
    reset?: boolean;
    run: any;
    children: OptionData[];
}

export interface FSMOptionData
{
    /** Starting state name. */
    start: string;
    transitions: {to: string; via: string; wildcard?: boolean}[];
    warning?: string;
}

/** Walks recursively through the option data, attempting to find data for an option with the specified id. */
export function findOptionData(data: OptionData, optionId: string): OptionData
{
    if (data.id === optionId)
        return data;

    for (var i = 0; !!data.children && i < data.children.length; i++)
    {
        var childResult = findOptionData(data.children[i], optionId);
        if (childResult)
            return childResult;
    }

    return null;
}

// TODO inline OptionData into OptionTree and create exported function to get ranOptions from walk of path

export interface OptionTree
{
    ranoptions: string[];
    path: OptionData;
}

export interface Orientation
{
    /** [w,x,y,z] */
    quaternion: number[];
}

export interface Particle
{
    /** [[x,y,theta,w],...] */
    particles: number[][];
    pnwsum: number;
}

export interface StaticJointData
{
    id: number;
    modelNumber: number;
    firmwareVersion: number;
    baud: number;
    returnDelayTimeMicroSeconds: number;
    angleLimitCW: number;
    angleLimitCCW: number;
    tempLimitHighCelsius: number;
    voltageLimitLow: number;
    voltageLimitHigh: number;
    maxTorque: number;
    statusRetLevel: number;
    alarmLed: string[];
    alarmShutdown: string[];
    torqueEnable: boolean;
    isEepromLocked: boolean;
}

export interface StaticHardware
{
    id: number;
    baud: number;
    firmwareVersion: number;
    modelNumber: number;
    returnDelayTimeMicroSeconds: number;
    statusRetLevel: number;
    joints: StaticJointData[];
}

export interface Timing
{
    cycle: number;
    timings: {[key:string]:number};
}

export interface WorldFrame
{
    /** [x,y,theta] */
    pos: number[];

    /** [x,y,z] */
    ball: number[];

    /** [[x,y,z],...] */
    goals: number[][];

    /** [[x1,y1,z1,x2,y2,z2],...] */
    lines: number[][];

    /** [[x,y],...] */
    visibleFieldPoly: number[][];

    /** [[x1,y1,x2,y2],...] */
    occlusionRays: number[][];
}
