#ifndef product_h
#define product_h

/*
    Product struct: the product struct used in mutiple files

    @variable count: specific product number
    @variable productType: the product type of the product (either A or B)
*/

typedef struct {
    int count;
    char productType[11];
} Product;
#endif