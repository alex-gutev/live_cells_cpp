#ifndef LIVE_CELLS_MERGED_OBSERVABLE_HPP
#define LIVE_CELLS_MERGED_OBSERVABLE_HPP

namespace live_cells {

    /**
     * A single observable which merges multiple observables.
     *
     * This class allows an observer to be added to multiple
     * observables simultaneously via a single observable object.
     */
    template <typename T, typename... Ts>
    class merged_observable : observable {
    public:

        /**
         * Create an observable that merges the observables @a first
         * and @a rest... into one.
         *
         * When add_observer() and remove_observer() are called, they
         * are applied to @a first and every observable in @a rest.
         *
         * @param first An observable
         * @param rest Obseravbles
         */
        merged_observable(T first, Ts... rest) :
            first(first),
            rest(rest...) {}

        void add_observer(observer::ref o) {
            first.add_observer(o);
            rest.add_observer(o);
        }

        void remove_observer(observer::ref o) {
            first.remove_observer(o);
            rest.remove_observer(o);
        }

    private:
        T first;
        merged_observable<Ts...> rest;
    };

    template <typename T>
    class merged_observable<T> : observable {
    public:

        merged_observable(T o) : first(o) {}

        void add_observer(observer::ref o) {
            first.add_observer(o);
        }

        void remove_observer(observer::ref o) {
            first.remove_observer(o);
        }

    private:
        T first;
    };


}  // live_cells

#endif /* LIVE_CELLS_MERGED_OBSERVABLE_HPP */
