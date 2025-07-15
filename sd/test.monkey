// Variable declarations
let n=narg(0);
let arg=args(0);



 
let fibonacci = fn(x) {
    if (x < 2) {
        return x;
    }

    return fibonacci(x - 1) + fibonacci(x - 2);
}
 
let str = "Comma,separated,values,daniel,quintero";
let arr = str_split(str, ",");
for (let i=0; i < len(arr); i=i+1) {
    puts(i, ": ", arr[i]);
}

let num = fibonacci(10);
puts("10th fibonacci number is: ", int(num));
 
puts("Integer value of true = ", int(true));
