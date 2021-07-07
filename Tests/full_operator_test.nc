main>
	println("Full Node Call intrinsic operations unit test");
	println("");
	
	println("Mathmatics test");
	assign( int1, 4);
	assign( int2, 5);
	println(add("4 + 5 = ", str(add(int1, int2))));
	println(add("5 - 4 = ", str(sub(int2, int1))));
	println(add("4 * 5 = ", str(mul(int1, int2))));
	println(add("10/ 2 = ", str(div(10, 2))));
	
	println("");
	println("");
	
	println("Logic test");
	println(add("4 << 1 = ", str(left_shift(4, 1))));
	println(add("16>> 1 = ", str(right_shift(16, 1))));
	println(add("3 ^ 12 = ", str(logical_xor(3, 12))));
	if(and(True, True)){
		println("1: True and True = True");
		println("Can it manage multi line ifs?");
	}

	if(not(and(True, False))){
		println("2: True and False = False");
	}

	if(not(or(False, False))){
		println("3: False or False = False");
	}	

	if(or(True, False)){
		link_to("other_node");
	}
>

other_node>
	println("Made it here because or worked, can successfully link to different nodes!");
>