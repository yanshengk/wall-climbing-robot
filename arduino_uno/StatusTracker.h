#ifndef STATUS_TRACKER_H
#define STATUS_TRACKER_H

template<typename T>
class StatusTracker {
private:
    T previous_;
    T current_;
public:
    // Constructors
    StatusTracker() = default;
    StatusTracker(const T& initialPrev, const T& initialCurr)
      : previous_(initialPrev), current_(initialCurr) {}

    // Getters
    T getPrevious() const { return previous_; }
    T getCurrent()  const { return current_;  }

    // Setters
    void setPrevious(const T& val) { previous_ = val; }
    void setCurrent (const T& val) { current_  = val; }
    
    // convenience: shift current→previous and apply new
    void update(const T& newVal) {
      previous_ = current_;
      current_  = newVal;
    }

    // Has the state changed?
    bool hasChanged() const { return previous_ != current_; }
};

#endif
