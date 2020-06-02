# Article 1
"I have learned two truths when it comes to debouncing:

    Buttons don't push themselves

    Humans can't do repeated button presses faster than 30 ms and keep up with their brain on how many times they have actually pressed it.

So my go-to to avoid tuning things for various mechanical buttons has been:

    As soon as the GPIO state changes to active (low in your example) you declare the button is pressed (due to rule #1 above).

    Ignore all further input on that button for 30 ms.

    Go back to step #1.

It allows for interrupt driven input and has zero delay between user action and input processing because you don't wait the debounce period before declaring it pressed. Its important to have that low delay in highly reactive control surfaces (games).

The downside is that it won't work if you need to pass regulatory ESD testing and don't have sufficient hardware protection. In that event, just validate the state is still the state after 5 ms and then go back to ignoring the input for another 25." - https://www.reddit.com/r/embedded/comments/gf74p8/reliable_user_input_with_unreliable_physical/

# Article 2

https://mrdrprofbolt.wordpress.com/2020/05/07/reliable-user-input-with-unreliable-physical-switches-a-simple-guide-to-debouncing/

# Article 3

Michael Barr_ Anthony J Massa - Programming embedded systems with C and GNU development tools (2006, O'Reilly) - page 207

# Article 4

https://www.reddit.com/r/embedded/comments/eqt5t3/too_many_buttons_best_way_to_do_it/