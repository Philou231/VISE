

enum ItemType{ITEM_UNDEFINED, ITEM_COMPONENT, ITEM_COMMENT, ITEM_MEASUREMENT};

//An item is an object that is tied to a position onto the board
//The easiest example is the Component which is an electrical/mechanical component physically present on the board and illustrated in the image
//Another example would be a comment added by the user onto the image of the board
//It only needs a position onto the image and the board and a layer to know the image(s) associated
struct Item
{
    ItemType type;
    std::string designator="";//Needed to uniquely identify each item

    Dbl_Rect posPickAndPlace={0.0, 0.0}; //In natural units (mm/inch)
    SDL_Rect pos={0, 0};//In pixel, relative to an image

    std::string comment="";//Inputted by user
};






enum CompState{COMP_UNDEFINED,COMP_FITTED,COMP_NOT_FITTED,COMP_ERROR};

struct Component : Item
{
    std::string layer=""; //Top, Bottom or other
    CompState state=COMP_UNDEFINED;
    bool polarized=true;

    Component() //Constructor
    {
        type = ITEM_COMPONENT;
    }
};

struct Comment : Item //When placed directly onto the board, a Comment is an item in and of itself
{
    std::string imagePath="";//To remember the image associated with this comment

    Comment() //Constructor
    {
        type = ITEM_COMMENT;
    }
};

struct Measurement : Item //Measurements give indication of position, angle and length
{
    std::string imagePath="";//To remember the image associated with this comment

    Dbl_Rect posPickAndPlace2={0.0, 0.0}; //In natural units (mm/inch)
    SDL_Rect pos2={0, 0};//In pixel, relative to an image

    double length;
    double angle;

    Measurement() //Constructor
    {
        type = ITEM_MEASUREMENT;
    }
};



