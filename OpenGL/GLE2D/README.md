# Ginzburg-Landau Simulation Using Finite Differences in 2D (WIP)

A first attempt at numerically solving the Ginzburg-Landau equation using finite difference methods. This simulation is done in a gauge that sets the scalar potential to zero, where for this particular gauge the Ginzburg Landau equations are referenced from [this article](https://inis.iaea.org/collection/NCLCollectionStore/_Public/27/046/27046815.pdf) (eq. 17-20). The numerical integration is done in an explicit fashion using [RK4](https://web.mit.edu/10.001/Web/Course_Notes/Differential_Equations_Notes/node5.html), with periodic boundary conditions. [Peierls substitution](https://en.wikipedia.org/wiki/Peierls_substitution) is used for handling the generalized momentum term that couples the spatial derivatives of ψ to the magnetic vector potential. 

## References

 - Wikipedia - [Ginzburg–Landau theory](https://en.wikipedia.org/wiki/Ginzburg%E2%80%93Landau_theory).

 - Pellé, Jaqueline, Kaper, Hans. [Gauges for the Ginzburg-Landau Equation of Superconductivity](https://inis.iaea.org/collection/NCLCollectionStore/_Public/27/046/27046815.pdf). IAEA Archives.

### Peierls substitution

-  Wikipedia - [Peierls substitution](https://en.wikipedia.org/wiki/Peierls_substitution)

-  Feynman R., Leighton R., Sands M. 
   [The Schrödinger Equation in a Classical Context: A Seminar on Superconductivity](https://www.feynmanlectures.caltech.edu/III_21.html).
   In <i>The Feynman Lectures on Physics. The New Millennium Edition</i>, Volume 3, chapter 21.
   Basic Books.

### Runge-Kutta, 4th Order

- Zeltkevic M. (1998, April 15). [Runge-Kutta Methods](https://web.mit.edu/10.001/Web/Course_Notes/Differential_Equations_Notes/node5.html). Lecture notes from <i>10.001 Introduction to Computer Methods</i>, Node 5.




