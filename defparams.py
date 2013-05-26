from boldpy.conf import *

CameraModule = Param()
agent = Param()
ambulator = Param()
datastreamer = Param()
field = Param()
gamestatereceiver = Param()
localiser = Param()
motion = Param()
motion.head = Param()
motion.walk = Param()
visialcortex = Param()
vision = Param()
vision.Ball = Param()
vision.Field = Param()
vision.Goal = Param()
vision.Line = Param()

CameraModule.ImageHeight = 240
CameraModule.ImageWidth = 320
CameraModule.RangeVerticalDegrees = 45.0
CameraModule.rangeHorizontalDegrees = 60.0

agent.u2dDevName = "/dev/ttyUSB0"
agent.useJoystick = False
agent.uniformNumber = -1

ambulator.maxHipPitch = 17.0
ambulator.maxHipPitchAtSpeed = 15.0
ambulator.minHipPitch = 13.0
ambulator.turnDelta = 1.0
ambulator.xAmpDelta = 3.0
ambulator.yAmpDelta = 3.0

datastreamer.tcpPort = 8080

field.CircleDiameter = 1.2
field.FieldSizeX = 6.0
field.FieldSizeY = 4.0
field.GoalAreaSizeX = 0.6
field.GoalAreaSizeY = 2.2
field.GoalSizeY = 1.5
field.OuterMarginMinimum = 0.7

gamestatereceiver.port = 3838

localiser.AngleErrorDegrees = 3
localiser.MinGoalsNeeded = 1
localiser.ParticleCount = 200
localiser.PositionErrorMillimeters = 0.03
localiser.RandomiseRatio = 0.05
localiser.RewardFallOff = 0.1
localiser.SmoothingWindowSize = 5
localiser.UseLines = 1

motion.head.bottom_limit = -25.0
motion.head.left_limit = 70
motion.head.pan_d_gain = 0.22
motion.head.pan_home = 0.0
motion.head.pan_p_gain = 0.1
motion.head.right_limit = -70
motion.head.tilt_d_gain = 0.22
motion.head.tilt_home = 10.0
motion.head.tilt_p_gain = 0.1
motion.head.top_limit = 40.0

motion.walk.arm_swing_gain = 1.5
motion.walk.balance_ankle_pitch_gain = 0.9
motion.walk.balance_ankle_roll_gain = 1.0
motion.walk.balance_hip_roll_gain = 0.5
motion.walk.balance_knee_gain = 0.3
motion.walk.dsp_ratio = 0.1
motion.walk.foot_height = 40
motion.walk.hip_pitch_offset = 13.0
motion.walk.pelvis_offset = 3.0
motion.walk.period_time = 600
motion.walk.pitch_offset = 0
motion.walk.roll_offset = 0
motion.walk.step_forward_back_ratio = 0.28
motion.walk.swing_right_left = 20.0
motion.walk.swing_top_down = 5
motion.walk.x_offset = -10
motion.walk.y_offset = 5
motion.walk.yaw_offset = 0
motion.walk.z_offset = 20

visialcortex.CameraFramePeriod = 5
visialcortex.DetectLines = 0
visialcortex.MinBallArea = 25

vision.Ball.hue = 10
vision.Ball.hueRange = 15
vision.Ball.saturation = 255
vision.Ball.saturationRange = 95
vision.Ball.value = 190
vision.Ball.valueRange = 95

vision.Field.hue = 71
vision.Field.hueRange = 20
vision.Field.saturation = 138
vision.Field.saturationRange = 55
vision.Field.value = 173
vision.Field.valueRange = 65

vision.Goal.hue = 40
vision.Goal.hueRange = 10
vision.Goal.saturation = 210
vision.Goal.saturationRange = 55
vision.Goal.value = 190
vision.Goal.valueRange = 65

vision.Line.hue = 0
vision.Line.hueRange = 255
vision.Line.saturation = 0
vision.Line.saturationRange = 70
vision.Line.value = 255
vision.Line.valueRange = 70
