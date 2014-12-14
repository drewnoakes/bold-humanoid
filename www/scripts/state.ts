/**
 * @author Drew Noakes http://drewnoakes.com
 */

export interface AgentFrame
{
    /** [x,y,z] */
    ball: number[];

    /** [[x,y,z],...] */
    goals: number[][];

    /** [[x,y,z],...] */
    teammates: number[][];

    /** [[x1,y1,z1,x2,y2,z2],...] */
    lines: number[][];

    /** [{p,a,t},{p,a,t},...] */
    junctions: {p: number[]; a: number; t: number}[];

    /** [[x,y],...] */
    visibleFieldPoly: number[][];

    /** [[x1,y1,x2,y2],...] */
    occlusionRays: number[][];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface AudioPowerSpectrum
{
    maxHertz: number;
    dbLevels: number[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export enum WalkStatus
{
    Stopped = 0,
    Starting = 1,
    Walking = 2,
    Stabilising = 3
}

export interface Walk
{
    running: boolean;
    status: WalkStatus;

    /** [x,y,turn] */
    target: number[];
    /** [x,y,turn] */
    current: number[];
    /** [x,y,turn] */
    delta: number[];
    phase: number;
    hipPitch: { target: number; current: number; delta: number };
    bodySwingY: number;
    bodySwingZ: number;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface JointControlData
{
    v: number;
    m: number;
    p: number;
    i: number;
    d: number;
}

export interface BodyControl
{
    cycle: number;
    joints: JointControlData[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface Body
{
    'motion-cycle': number;
    angles: number[];
    /** Differences between current and control. */
    errors: number[];
    /** Centre of mass [x,y,z] */
    com: number[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface PlayerData
{
    penalty: string;
    penaltySecondsRemaining?: number;
}

export interface TeamData
{
    num: number;
    isBlue: boolean;
    score: number;
    players: PlayerData[];
}

export interface Game
{
    playMode: string;
    packet: string;
    playerPerTeam: number;
    isFirstHalf: boolean;
    nextKickOffTeamIndex: boolean;
    isPenaltyShootOut: boolean;
    isOvertime: boolean;
    isTimeout: boolean;
    lastDropInTeamColor: number;
    secSinceDropIn: number;
    secondsRemaining: number;
    secondsSecondaryTime: number;
    myTeam: TeamData;
    opponentTeam: TeamData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface LabelCount
{
    labels: {name:string; id:number; count:number}[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface LabelTeacher
{
    selectedRange: {hue: number[]; sat: number[]; val: number[]};
    selectedDist: {hue: number[]; sat: number[]; val: number[]};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface LED
{
    /** [r,g,b] as bytes */
    eyeColor: number[];
    /** [r,g,b] as bytes */
    foreheadColor: number[];
    /** [b,b,b] as booleans */
    led: number[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export enum LogLevel
{
    Trace = 0,
    Verbose = 1,
    Info = 2,
    Warning = 3,
    Error = 4
}

export interface LogMessage
{
    lvl: LogLevel;
    scope: string[];
    msg: string;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface MessageCount
{
    gameControllerMessages: number;
    ignoredMessages: number;
    sentTeamMessage: number;
    receivedTeamMessages: number;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface BodySectionTaskData
{
    module: string;
    priority: number; // TODO is this an enum?
    committed: boolean;
    selected: boolean;
}

export interface MotionTask
{
    head: BodySectionTaskData;
    arms: BodySectionTaskData;
    legs: BodySectionTaskData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface Odometry
{
    /** [x,y,z] */
    translation: number[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export enum PlayerRole
{
    /// Robot is running, but paused or otherwise indisposed and should not be
    /// considered as actively on the team at present.
    Idle = 0,

    /// Robot is acting as the keeper, determined by uniform number.
    /// By convention, the keeper has unum 1.
    Keeper = 1,

    /// Robot is positioning to receive a pass towards the goal.
    Supporter = 2,

    /// Robot is claiming possession of the ball and advancing it towards the
    /// opponent's goal.
    Striker = 3,

    /// Robot is positioning so as to block an opponent's advance towards
    /// our goal.
    Defender = 4,

    /// Robot is acting as a keeper during a penalty shootout.
    PenaltyKeeper = 5,

    /// Robot is acting as a striker during a penalty shootout.
    PenaltyStriker = 6,

    /// Robot role is unknown.
    /// This status will not be transmitted by the Bold Hearts as our agents
    /// default to striker/keeper in the absence of sufficient information to
    /// decide otherwise.
    Other = 7
}

export function getPlayerRoleName(role: PlayerRole)
{
    switch (role)
    {
        case PlayerRole.Idle: return "Idle";
        case PlayerRole.Keeper: return "Keeper";
        case PlayerRole.Supporter: return "Supporter";
        case PlayerRole.Striker: return "Striker";
        case PlayerRole.Defender: return "Defender";
        case PlayerRole.PenaltyKeeper: return "PenaltyKeeper";
        case PlayerRole.PenaltyStriker: return "PenaltyStriker";
        case PlayerRole.Other: return "Other";
        default: return "Unknown";
    }
}

export enum PlayerActivity
{
    /// Robot is moving to a supporting or defending position.
    Positioning = 0,

    /// Robot is moving towards the ball, either as a striker or a defender.
    ApproachingBall = 1,

    /// Robot has possession of the ball and is attacking the opponent's goal,
    /// as a striker.
    AttackingGoal = 2,

    /// Robot is not taking any action, as keeper, supporter, or defender.
    Waiting = 3,

    /// Robot activity is unknown.
    /// This status will not be transmitted by the Bold Hearts as the other
    /// enum members sufficiently cover our activities. This value may be
    /// seen when playing in a mixed team, however.
    Other = 4
}

export function getPlayerActivityName(activity: PlayerActivity)
{
    switch (activity)
    {
        case PlayerActivity.Positioning: return "Positioning";
        case PlayerActivity.ApproachingBall: return "ApproachingBall";
        case PlayerActivity.AttackingGoal: return "AttackingGoal";
        case PlayerActivity.Waiting: return "Waiting";
        case PlayerActivity.Other: return "Other";
        default: return "Unknown";
    }
}

export enum PlayerStatus
{
    /// Robot is not doing anything, or is incapable. It may have fallen, or
    /// the game may be in a play mode that does not permit motion (eg. Set.)
    /// The activity should be set to Waiting and the role set to Idle.
    Inactive = 0,

    /// Robot is active and able.
    Active = 1,

    /// The robot has been penalised and is not permitted to take any action.
    Penalised = 2
}

export function getPlayerStatusName(status: PlayerStatus)
{
    switch (status)
    {
        case PlayerStatus.Inactive: return "Inactive";
        case PlayerStatus.Active: return "Active";
        case PlayerStatus.Penalised: return "Penalised";
        default: return "Unknown";
    }
}

export interface PlayerData
{
    /** The player's uniform number. */
    unum: number;
    /** The player's team number. */
    team: number;
    /** Whether this object represents the robot reporting this data. */
    isMe: boolean;
    activity: PlayerActivity;
    status: PlayerStatus;
    role: PlayerRole;
    /** The agent's estimate of its position in the world frame: [x,y,theta] in [m,m,rads] */
    pos: number[];
    /** Between 0 and 1 */
    posConfidence: number;
    /** [x,y] in metres, relative to the agent's frame. */
    ballRelative: number[];
//    /** Between 0 and 1 */
//    ballConfidence: number;
    /** Time that the message was received according to the agent's own clock, in milliseconds. */
    updateTime: number;
}

export interface Team
{
    players: PlayerData[];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    if (!data)
        return null;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface Orientation
{
    /** [x,y,z,w] */
    quaternion: number[];
    pitch: number;
    roll: number;
    yaw: number;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface Particle
{
    /** [[x,y,theta,w],...] */
    particles: number[][];
    pnwsum: number;
    pnwsumsmooth: number;
    uncertainty: number;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface StationaryMap
{
    balls: AveragePosition[];
    goalPosts: AveragePosition[];
    goals: GoalPosition[];
    keepers: AveragePosition[];
    kicks: Kick[];
    openField: OpenFieldData;
}

export interface AveragePosition
{
    pos: number[];
    count: number;
}

export interface GoalPosition
{
    post1: number[];
    post2: number[];
    label: GoalLabel;
}

export enum GoalLabel
{
    Unknown = 0,
    Ours = 1,
    Theirs = 2
}

export interface Kick
{
    id: string;
    /** 2D vector of kick's estimated end position. */
    endPos: number[];
    /** Whether this kick is in the correct direction. */
    onTarget: boolean;
}

export interface OpenFieldData
{
    divisions: number;
    slices: OpenFieldSlice[];
}

export interface OpenFieldSlice
{
    angle: number;
    near?: {
        dist: number;
        count: number;
    };
    far?: {
        dist: number;
        count: number;
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface Timing
{
    cycle: number;
    fps: number;
    timings: {[key:string]:number};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export interface BalanceOffset
{
  hipRoll: number;
  knee: number;
  anklePitch: number;
  ankleRoll: number;
}

export interface Balance
{
    offsets: BalanceOffset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

export enum Frame
{
    Camera = 1,
    Agent = 2,
    World = 3
}

export enum DrawingItemType
{
    Line = 1,
    Circle = 2,
    Polygon = 3
}

export interface DrawingItem
{
    frame: Frame;
    type: DrawingItemType;
}

export interface StrokeableDrawingItem extends DrawingItem
{
    rgb?: number[];
    a?: number;
    w?: number;
}

export interface FillableDrawingItem extends DrawingItem
{
    fa?: number;
    sa?: number;
    frgb?: number[];
    srgb?: number[];
    w?: number;
}

export interface LineDrawing extends StrokeableDrawingItem
{
    p1: number[];
    p2: number[];
}

export interface CircleDrawing extends FillableDrawingItem
{
    c: number[];
    r: number;
}

export interface PolygonDrawing extends FillableDrawingItem
{
    p: number[][];
}

export interface Drawing
{
    items: DrawingItem[];
}
