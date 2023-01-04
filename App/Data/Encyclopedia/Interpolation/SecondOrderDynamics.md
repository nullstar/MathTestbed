[DEMO](OpenWindow SecondOrderDynamics)

# SECOND ORDER DYNAMICS
This is a method of interpolating values using a second order dynamic system which accounts for both the velocity and acceleration of the value.  This results in interpolation where both the absolute value and the rate of change of the value alter smoothly over time giving a very natural looking interpolation that is suitable for many realtime use cases.

The damping and initial response characteristics of the method can be altered allowing for interpolations that display overshoot and oscillation around the target value or a pre-emptive reaction to the interpolation.  This makes it great to use for immitating spring like or imperfectly damped motions or for procedural animations.

The method output is stable to variations in delta-time so is safe to use in variable delta time environments.  Additionally it has safety features built in to prevent numerical instability and jitter in cases where the control parameters are tuned to the extreme or the delta time grows excessively large.

A clamp function can be supplied to the methods update function to limit the rate of change of the interpolated value.  This allows the speed of the value to be kept within user specified limits which is useful in a lot of realtime scenarios.  For example excessively high rates of change might be undesirable when interpolating properties of a camera since that would create disorientation.

Given the realiability, flexibility, and natural look and quality of the output, I highly recommend use of this method for smooth interpolation in most use cases.