[DEMO](OpenWindow RootFinding)

# ROOT FINDING

Root finding algorithms are used for finding where a continuous function equals zero, otherwise known as the roots of that function.  This can have many uses in game development.  Many cases can be solved using analytical methods where all the roots of a function can be found exactly.  Where analytical methods can't be used, maybe due to the form of the equation or where your system can't be expressed by a simple equation, iterative methods can be used instead.  These methods make progressive guesses that get closer and closer to the real root from an initial guess.  They do have some limitation however.  They can only find one root at a time, they require a reasonable initial guess to the value of the root, and they aren't garantueed to succeed. 


## ROOT FINDING METHODS

### QUADRATIC

The quadratic method finds the roots of second order polynomial functions.  That is functions of the form:

f(x) = a.x^2 + b.x + c

In these cases there can be 0, 1, or 2 roots to the function.

One example of this is finding the minimum and maximum values presented by a hermite spline segment.  The min and max values will either lie on a control point of the segment or be one of the top most values of the segments hills or the bottom most values of the segments valleys, known as the turning points.  Finding the bounds of the control points is trivial but finding the turning points is not so straight forward but can be solved using root finding methods.

The gradient of the segment is always zero where turning point occur, and non-zero at all other locations.  Therefore if we find the roots of the function which represents the curves gradient we find the tuning points.  The gradient function for a hermite spline segment, i.e. the derivative of the spline function, is a second order polynomial.  Were therefore just need to plug the polynomial coefficients (a, b, and c) into the quadratic root finding function and this will return the positions along the segment where the turning points are.  You can then find the bounds of the control points and turning points to find the min and max values of the segment.

The demo shows the turning points of the segment along with whether they are a maximal turning point (hill) or a minimal turning point (valley) which can be inferred from the splines second derivative.  (You can drag the control nodes in the demo to change the shape of the spline segment).

The quadratic method is fully analytical and thus is garaunteed to always find the exact roots when they exist.

### CUBIC

The cubic method is similar to the quadratic method above but finds the roots of third order polynomial functions, i.e. functions of the form:

f(x) = a.x^3 + b.x^2 + c.x + d

In these cases there can be 0, 1, 2, or 3 roots to the function.

For example say you want to find all the positions at which a hermite spline segment has a value of zero.  This is the same as finding the roots of the hermite spline equation which is a cubic polynomial and hence can be solved using the cubic root finding method.  The demo shows all the found zero crossing points for the segment using this method.  You could find the crossing points for any value other than zero by translating the segment before applying the cubic method.

If you look at the implemetation for the cubic method you'll see that it is not fully analytical but also involves an iterative step.  This is because there isn't a formula methods for solving cubics, thus we use an iterative step initially to find one of the roots.  This root can then be used to reduce the third order polynomial to a second order form using factor theorem.  This reduced polynomial can then be solved using the above quadratic root finding method to find the remaining roots.  For the iterative step we use a Newton Raphson method which is described below.

Note that methods for finding the roots of higher order polynomials can also be constructed but it is rare in game development to encounter something higher than third order.  Higher order methods would follow a similar pattern using iterative methods to find roots and factor theorem to use those roots to reduce the function.

### NEWTON RAPHSON

Newton Raphson is a fully interative root finding method.  It works from an initial guess to the position of the root and then iteratively moves closer to the real root by evaluating the function and the functions gradient and then moving "downhill" for the next guess.  This continues until the guess is within an agreed margin of error in which case the root is considered found, or until a maximum number of iterations is reached in which case the root is considered to not exist or to be unreachable from the initial guess.

Iterative methods like this rely on a good initial guess to work correctly.  A good initial guess should meet the following conditions:

- It is closer to the root you wish to find than any other root that exists
- It is closer to the root you wish to find that any turning point of the function (local maxima, minima, or point of inflection)

If the initial guess is not good then it can result in the method failing to converge and the max interation count will be reached without finding the root.  For the Cubic method above where we use the Newton Raphson method initially to find the first root, we therefore need to handle what happens if the iterative method fails.  It starts out with an initial guess half way along the segment but that is not garaunteed to be a good guess and return a root.  If it fails then the first control point is used followed by the second if that then fails.  For Hermite segments at least one of these is garaunteed to be closer to a crossing point than a turning point if any crossing points exist.

Note that iterative methods will also only find a single root each time, the one closest to the initial guess assuming the method succeeds.  If you wish to use iterative methods to find multiple roots you must therefore run it once for each root you wish to find with a good initial guess for each root.

For the Newton Raphson method to be used, the first derivative of the function you wish to find roots for must be known.  If it is not known then the Secant method below can be used instead.

### SECANT METHOD

This is essentialy the same as the Newton Raphson method except that the function derivative need not be known.  Instead two initial guesses must be supplied and the method uses the slope implied by connecting together the two guesses to improve those guesses for the next iteration.  All the same limitations apply.  Both guesses must be "good", and only one root can be found at a time.