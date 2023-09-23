#ifndef DONUT_OVERLOADED_HPP
#define DONUT_OVERLOADED_HPP

namespace donut {

/**
 * Visitor type for combining callable objects, such as lambdas, into an
 * overload set.
 *
 * \tparam Functors callable object types to combine.
 */
template <typename... Functors>
struct Overloaded : Functors... {
	using Functors::operator()...;
};

template <typename... Functors>
Overloaded(Functors...) -> Overloaded<Functors...>;

} // namespace donut

#endif
