[DEMO](OpenWindow ExponentialDecay)

# EXPONENTIAL DECAY
Exponential decay occurs when the value decreases at a rate proportional to its current value such that it approaches zero over time.  In other words each update the value is multiplied by a decay factor between 0 and 1.  This results in large values decreasing quickly and small values decreasing slowly thus creating a smooth decay towards 0.  

To allow for non zero and variable targets we apply the decay to the difference between the value and the target rather than directly to the value itself.

## FIXED EXPONENTIAL DECAY
This is the most basic form of exponential decay where the decay factor is kept constant and applied to the value each udpate.  This has the advantage of being the most CPU performant option.  It's also easy to tune since the resulting curvature changes linearly with the supplied decay factor.

The disadvantage of this method is that the decay is sensitive to variations in delta time which creates inconsistent results when used in a variable delta time environment.  I therefore only recommend its use where delta time is fixed.

## INDEPENDENT EXPONENTIAL DECAY
This is an attempt to fix the shortcomings of fixed exponential decay.  Instead of the control parameter being the decay factor, a "scale per second" parameter is used.  The decay factor itself is no longer a fixed quantity but is calculated each update factoring in the delta time to create an amount of decay that matches the control parameter.  This makes it safe to use in variable delta time environments since the output is stable with delta time variations.  The control parameter is also more logical and therefore easier to understand when tuning.

There are a couple of disadvantes however.  Firstly it is not as performant as the fixed method since the decay factor calculation involes evaluating a CPU expensive "powf" function.  Secondly the resulting exponential curvature doesn't change linearly with the control parameter.  Trying to create steeper decays requires increasingly smaller values of the control parameter to the point where it is very difficult to finely control the decay rate and you may even run into floating point accuracy problems.  This can make tuning the decay extremely difficult or even impossible for very steep settings.

For these reasons I'd recommend only using this method where having a more logical control parameter is critical.  For most applications you will be better served by the fixed or normalised methods.

## NORMALISED EXPONENTIAL DECAY
This method solves the largest disadvantages of the indepedent method whilst maintaining it's most important advantages.  It can be observed that by scaling the time component of an indepedent decay curve with a fixed control parameter it can be made to fit any other unscaled indepedent decay curve with any control parameter value.  The kicker is that the resulting curvature does change linearly with scaled time and therefore is much easier to tune and to create steep decays without running into floating point accuracy issues.

This method uses the same math as the independent curve but with the addition of a scale factor applied to the delta time.  The "scale per second" is kept constant and defaulted at a reasonable value that allows the curve to be tuned without floating point accuracy problems.  The control parameter is now a "damping factor" that determines how the delta time is scaled.

The result is that this method is stable in variable delta time environments and can be tuned linearly.  It still has the disadvantage of being more expensive than the fixed method however.  I therefore recommend using this method for most use cases where delta time is variable.