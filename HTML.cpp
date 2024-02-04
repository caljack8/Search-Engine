#include "HTML.h"

//constructor
HTML::HTML(const std::string& aTitle, const std::string& aURL, const std::string& adescription,
		const std::list<std::string> alinks, const std::string acontent){

	title = aTitle;
	URL = aURL;
	description = adescription;
	links = alinks;
	content = acontent;
	pagescore = 0;
}