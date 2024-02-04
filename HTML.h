#ifndef HTML_H
#define HTML_H
#include <string>
#include <vector>

class HTML{
public:

	//constructor
	HTML(const std::string& aTitle, const std::string& aURL, const std::string& adescription,
		const std::list<std::string> alinks, const std::string acontent);

	//Accesors
	std::string get_content() const {return content;}
	int get_pagescore() const {return pagescore;}
	std::list<std::string> get_links() const {return links;}
	std::string get_filename() const {return URL.substr(URL.rfind("/")+1);}

	//changer
	void give_score(int ascore);

private:

	//member variables
	std::string title;
	std::string URL;
	std::string desription;
	std::string content;
	std::list<std::string> links;
	int pagescore;
};

bool operator< (const HTML& a, const HTML& b) {return a.get_pagescore() < b.get_pagescore();}

#endif