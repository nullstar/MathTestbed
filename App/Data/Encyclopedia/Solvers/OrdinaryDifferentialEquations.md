[DEMO](OpenWindow OrdinaryDifferentialEquations)

# ORDINARY DIFFERENTIAL EQUATIONS

An ODE is an equality involving a function and it's derivatives, for example given F, a function of x, y, and derivatives of y, an ODE might look like:

y" = F(x, y, y')

These often appear in equations of motion and so pop up in game development all the time.  For example the above ODE could represent the acceleration of a particle (y") expressed as a function of the particles position (y), speed (y'), and the current time (x).  In game development we will often want to extrapolate such equations forwards in time to determine the state of a system from a known starting state.  In the above example we would wish to know the next state of the particle each frame given the frame's delta-time, the particles previous state, and the function of motion for the particle.  To do this we can use a range of different iterative numerical methods to approximate the future state.

## EXPLICIT METHODS

Explicit methods use only the known state at the current time to estimate the state of the system at a future point in time.  As as result they are mathematically simple which makes them easy to implement and efficient to process.  One example of these methods is known as the Explicit Euler method which is represented by the following equation.  

y1 = y0 + h * F'(t0, y0)

What this says is that the future state of the system (y1) is the current state (y0) plus the step time (h) multipled by a function of the state and time which returns the derivative of the state.

Imagine a particle with position 'x'.  Each frame the application calls a function which applies a velocity to this particle based on its position.  We'll call this function ApplyVelocity.  Note that this function is returning the derivative to the position, i.e. velocity.  To approximate the future position of the particle at the end of the frame which lasts 'dt' seconds using the Explicit Euler method would look like this:

xNext = x + dt * ApplyVelocity()

This can be expanded to include systems which track multiple derivatives of the state.  For example say that our particle has mass 'm' and our function instead adds forces to this particle each frame so we'll call it ApplyForce() instead.  The state of the particle has also been expanded to track it's velocity 'v' each frame.  Given that force divided by mass gives acceleration which is the derivative of velocity, and velocity is the derivative of position, the explicit euler update method for this system would look as follows:

xNext = x + dt * v
vNext = v + dt * ApplyForce() / m

The Explicit Euler method is the simplest of the explicit methods and is known as first order since it only takes one step to approximate the future state.  This makes it very efficient but also makes it generate a lot of error.  In addition it also over-shoots the correct state with its approximations which results in instability unless the system has sufficient damping.  The Explicit Euler method also reacts poorly to increases to the stiffness of the system or reduction in step size which reults in the error it generates increasing rapidly.

One way to reduce error and improve stability is to use higher order explicit methods.  These are methods that blend together multiple gradients to the equation to create more accurate approximations.  These therefore require more computing power since they have to make multiple evaluations but can significantly reduce error and improve stability.  I've provided a 2nd order solution called the Explicit Midpoint method, and a 4th order solution which is more accurate but costly again called the Explicit RK4 method.  The Explicit Midpoint method has similar properties to the Explicit Euler method but generates error more slowly and can handle stiffer equations and lower step times before becoming unstable.  The Explicit RK4 method acts slightly different in that except for very stiff equations it has a tendancy to under-shoot the correct state.  This makes it a lot more stable but also makes systems lose energy over time when this method is used which could make it undesirable. 

In general, despite their cheapness, explicit methods can be unsuitable for game development because instablity and too much variance to changing delta-time are often unacceptable.

## IMPLICIT METHODS

What happens when the accuray or stability of explicit methods isn't enough?  This can happen when dealing with particularly stiff systems or when you can't reduce the step time any further.  In this case implicit methods can be used. These methods use both the current and future state of the system to extrapolate which makes them significantly more stable and capabale of dealing with large step times but at much greater complexity.  The implicit version of the Euler method therefore looks like this:

y1 = y0 + h * F'(t1, y1)

Note the derivative function uses the future state rather than the current state.  As with explicit methods, higher order implicit methods also exist which increase accuracy further for increased cost.

Solving these functions where the unknowns appear on both sides requires use of iterative root-finding solvers.  For example the Newton-Raphson method can be used if the derivative can be analytically calculated.  In many game applications this isn't feasible however due to their complex and chaotic nature and also the desire to not hard code derivative functions to maintain code flexibility.  In that case a Secant method can be used which is a Newton-Raphson like method that estimates the derivative function.

The problem for game development is that implicit methods can become prohibitively expensive and are more difficult to work with and understand.  In addition the need to use estimating root solvers can create further points of innacuracy and instability given floating point error and the need to keep iteration counts low for performance.  This somewhat negates the reason to use implicit methods in the first place.  Therefore I won't be deep diving into fully implicit methods.

## SEMI-IMPLICIT METHODS

Semi-impicit methods allow us to blend together the cheapness and simplicity of explicit methods with much of the added accuracy and stability of implicit methods.  For systems with n derivatives, they work by using an explicit step to determine the nth derivative.  Each lower order derivative can then be calculated implicitly using the calculated value of the derivative above it.  In our particle example, we would calculate the future velocity state explicitly, and then use the future velocity state to calculate the next position state implicitly.  Therefore the semi-implicit Euler method looks like this:

vNext = v + dt * ApplyForce() / m
xNext = x + dt * vNext

As with the other method types, higher order versions of semi-implicit methods exist for improved accuracy and stability at the cost of performance.  I've provided implementations for Velocity Verlet which is a second order method, and Ruth4 which is a fourth order method.  Note that Velocity Verlet and Ruth4 are special cases for 2nd order differential equations.  To date I'm not aware of higher order semi-implicit methods that can be applied generically for any order of equation.  The majority of problems you encounter in game development will be 2nd order equations however since you are aften applying forces to entites whilst tracking their velocity and position.  If you have a problem for which the equations aren't 2nd order then stick to semi-implicit euler or use a higher order explicit method if needed.

Semi-implicit methods have singificantly better behaviour for really no more processing cost relative to fully explicit methods.  I therefore recommend using these for most cases.

## COUPLED EQUATIONS

You won't always be dealing with things like particles in isolation and often problems will present themselves as systems of coupled differential equations.  For example imagine two particles connected together via a spring.  The equations which govern each particles movement are now coupled since the force acting on each particle will depend on the length of the spring and hence the position of the other particle.  For example for two particles A and B:

forceA = (xB - xA) * springConstant
forceB = (xA - xB) * springConstant

You could choose to update each particle individually and for each particle simply treat the other coupled particle's position as fixed for the sake of calculating the spring force.  This would work but bypasses many of the benefits of using more advanced ODE methods and would result in accuracy and stability issues similar to if you had used explicit euler update methods.  Forunately any number of coupled equations can be solved togehter as a system using the ODE solving methods described above by treating the state as a vector that includes the values of all coupled equations.  For example the semi-implicit euler method would become:

VNext = V + dt * F(X, V)
XNext = X + dt * VNext

For our particle example V represents a vector that includes the velocity of both particles, V = {vA, vB}, and X presents the particles positions, X = {xA, xB}.  VNext and XNext are similar vectors that hold the future state.  Note that the "ApplyForce() / m" term has been replaced for a more generic derivative function F.  This function takes the position and velocity state of the system and returns the acceleration of the system.  For our particle example, if mA and mB are the masses of the particles, this function in psuedo code looks like:

F(X, V)
{ 
	return { (X[1] - X[0]) * springConstant / mA,
		(X[0] - X[1]) * springConstant / mB }
}

In other words this function calculates the force acting on each particle and divides that by the particle's mass to return a vector of accelerations.  This same derivative function F could be plugged into any of the ODE methods including higher order methods such as Velocity Verlet and Ruth4.

To model larger systems of coupled equations you only need to expand the size of the array.  For example a vehicle's suspension system might contain a vector state with 5 entries, one for each of the 4 wheels and 1 for the chassis body since they are all coupled together.