/**
 * @author Drew Noakes https://drewnoakes.com
 */

export interface KeyFrame
{
    pauseCycles?: number;
    moveCycles: number;
    values: JointValues;
}

export interface JointValues
{
    [key: string]: number;
}

export interface Stage
{
    repeat?: number;
    speed?: number;
    pGains?: JointValues;
    keyFrames: KeyFrame[];
}

export interface MotionScript
{
    name: string;
    controlsHead?: boolean;
    controlsArms?: boolean;
    controlsLegs?: boolean;
    stages: Stage[];
}
