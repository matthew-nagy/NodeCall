#main is just used to set up preliminary variables
main>
    assign(first, 0.0);
    assign(second, 1.0);
    assign(stringTest, "Writeout time!");
    assign(currentIndexOfSecond, 0x02);

    #Send over to loop for the actual looping part
    link_to("loop");
>

#The loop is where the numbers are found
loop>
    assign(result, add(first, second));
    assign(first, second);
    assign(second, result);
    assign(currentIndexOfSecond, add(currentIndexOfSecond, 1));
    println("Fibonacci num of index ", currentIndexOfSecond, " is ", second);

    #If you have reached the 100th number (in binary for test purposes), you can end the program
    if(equality(currentIndexOfSecond, 0b01100100)){
        end();
    }
    #Otherwise go back to the top and run it again
    link_to("loop");
>