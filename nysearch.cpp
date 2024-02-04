#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <regex>

// function to parse an HTML file and extract links to local files
std::list<std::string> extractLinksFromHTML(const std::string& fileContent){
    std::list<std::string> links;
    // regular expression to match href attributes in anchor tags
    std::regex linkRegex("<a\\s+[^>]*href\\s*=\\s*['\"]([^'\"]+)['\"][^>]*>");
    std::smatch match;

    // search for links in the HTML content
    std::string::const_iterator start = fileContent.cbegin();
    while (std::regex_search(start, fileContent.cend(), match, linkRegex)){
        if (match.size() > 1) {
            links.push_back(match[1].str());
        }
        start = match.suffix().first;
    }

    return links;
}

//finds the title in the content of the HTML file
std::string title_find(const std::string& fileContent, std::string& title){
	int Title_start = fileContent.find("<title>")+7;
	int Title_end = fileContent.find("</title>", Title_start);
	while(Title_start != Title_end){
		title+=fileContent[Title_start];
		Title_start++;
	}
	return title;
}

//finds the URL in the content of the HTML file
std::string URL_find(const std::string& fileContent, std::string& URL){
	int URL_start = fileContent.find("<h1>")+4;
	int URL_end = fileContent.find("</h1>", URL_start);
	while(URL_start != URL_end){
		URL+=fileContent[URL_start];
		URL_start++;
	}
	return URL;
}

//finds the Description in the content of the HTML file
std::string description_find(const std::string& fileContent, std::string description){
	int description_start = fileContent.find("content=\"")+9;
	int description_end = fileContent.find("\">", description_start);
	while(description_start != description_end){
		description+=fileContent[description_start];
		description_start++;
	}
	return description;
}

//finds the body in the content of the HTML file
std::string body_find(const std::string& fileContent, std::string body){
	int body_start = fileContent.find("<body>")+6;
	int body_end = fileContent.find("</body>", body_start);
	while(body_start != body_end){
		body+=fileContent[body_start];
		body_start++;
	}
	return body;
}

//finds the snippet for the keywords
std::string snippet_find(const std::string& fileContent, const std::vector<std::string>& keywords, std::string& snippet){
	
	//makes the phrase
	std::string phrase = "";
	for(unsigned int i = 0; i < keywords.size(); i++){
		if(i == keywords.size()-1){
			phrase += keywords[i];
		}
		else{
			phrase += (keywords[i]+" ");
		}
	}

	//gets the body of the content
	std::string body;
	body = body_find(fileContent, body);

	//declares start postion
	int start_pos;

	//gets starting position if the phrase is in there
	if(body.find(phrase) != std::string::npos){
		start_pos = (body.rfind('.', body.find(phrase))+1);
	}

	//gets the starting position if the phrase is not in there
	else{
		start_pos = (body.rfind('.', body.find(keywords[0]))+1);
	}

	//goes to the next character that isn't a white space
	while(std::isspace(body[start_pos]) != 0){
		start_pos++;
	}

	//makes the snippet of the next 120 characters
	for(int i = 0; i < 120; i++){
		snippet+=body[start_pos+i];
	}

	return snippet;
}

//goes back directories if it needs to
void back_step(std::string& link, std::string& linkstart){
	if(link.find("../") != std::string::npos){
		linkstart.erase(linkstart.length()-1);
		linkstart.erase(linkstart.find_last_of("/")+1);
		link.erase(link.find("../",3));
		back_step(link, linkstart);
	}
	return;
}

//goes through the first document and finds the links to other documents if there are any
void web_crawl(const std::string& fileContent, std::string& link,
	std::map<std::string, std::string>& all_files){

	//gets the URL to use for the directory
	std::string URL;
	URL = URL_find(fileContent, URL);

	//gets the directory from the URL;
	std::string directory;
	size_t lastSlashPos = URL.find_last_of('/');
	if (lastSlashPos != std::string::npos) {
		directory = URL.substr(0, lastSlashPos + 1);
	}

	//gets the file from the link
	std::string file = link.substr(link.rfind("/")+1);
	
	//if the file is already in the map then skip to the neext link
	if(all_files.find(file) != all_files.end()){
		return;
	}

	//if the link wants to go up a directory this does that and changes the link to fit the directory
	back_step(link, directory);

	//makes the new file link and gets the content from it
	std::string newfile = directory+link;
	std::ifstream infile(newfile);
	std::string newfileContent((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

	//adds the file to the map of files and it's content
	all_files[file] = newfileContent;

	//calls web crawl on the links of the new files  links
	std::list<std::string> links = extractLinksFromHTML(newfileContent);

	if(links.size() == 0){
		return;
	}

	std::list<std::string>::iterator link_it = links.begin();
	while(link_it != links.end()){
		web_crawl(newfileContent, *link_it, all_files);
		link_it++;
	}
   	return;
}

//finds the keyword density socre for each keyword in document and adds them for the documents total score
double total_keyword_density(std::map<std::string, std::string>& all_files,
	const std::string& keyword){

	//variables for the total amount of keywords and document length
	double total_keyword_count = 0;
	double total_document_lengths = 0;

	//goes through the documents and gets the key total density for this key word
	std::map<std::string, std::string>::iterator itr = all_files.begin();
	while(itr != all_files.end()){

		//add to the total document length
		total_document_lengths += itr->second.length();
		int start_pos = itr->second.length();
		while(itr->second.rfind(keyword, start_pos) != std::string::npos){
			total_keyword_count++;
			start_pos = itr->second.rfind(keyword, start_pos)-1;
		}

		itr++;
	}

	//return the total page density
	return total_keyword_count/total_document_lengths;
}

double keyword_density(const std::string& fileContent, const std::string& keyword, double tot_dens){

	//count of the keywords in this document
	double keyword_count = 0;

	//gets the denominator for finding the density score
	double denominator = (fileContent.length()*tot_dens);

	//adds each time the keyword is found
	int start_pos = fileContent.length();
	while(fileContent.rfind(keyword, start_pos) != std::string::npos){
		keyword_count++;
		start_pos = fileContent.rfind(keyword, start_pos)-1;
	}

	return keyword_count/denominator;
}

//gets the backlinks score based on the files pointing to it
double backlinks(const std::string& filename, std::map<std::string, std::string>& all_files){

	//intializes the score for the file
	double backscore = 0;

	//checks all the files for links to thois file
	std::map<std::string, std::string>::iterator file_itr = all_files.begin();
	while(file_itr != all_files.end()){

		//the outgoing links in the file and it's iterator
		std::list<std::string> links = extractLinksFromHTML(file_itr->second);
		std::list<std::string>::iterator link = links.begin();

		//check if any of the links goes to filenames file
		while(link != links.end()){
			std::string this_link = *link;
			if(filename == this_link.substr(this_link.rfind("/")+1)){
				backscore += double(1/double(1+links.size()));
				break;
			}
			link++;
		}

		file_itr++;
	}
	return backscore;
}

//searches based on the given key words
bool regular_search(const std::string& fileContent, const std::vector<std::string>& keywords){

	//puts the body of the content into a variable
	std::string body;
	body = body_find(fileContent, body);

	//for every keyword found this increases
	unsigned int found = 0;

	//goes through the key words and tries to find the keyword in the body
	for(unsigned int i = 0; i < keywords.size(); i++){

		unsigned int start_pos = 0;

		while(start_pos <= body.length() && start_pos != std::string::npos){

			if(body.find(keywords[i], start_pos) == std::string::npos){
				return false;
			}

			//if the keyword is found without white space then add to found counter and continue
			else if(std::isspace(body[body.find(keywords[i], start_pos)-1]) == 0 ||
			std::isspace(body[body.find(keywords[i], start_pos)+keywords[i].length()]) == 0){

				//try the next section of the body
				start_pos = body.find(keywords[i], start_pos)+keywords[i].length();
				continue;
			}

			found++;
			break;
		}
	}

	//if haven't found all the keywords return fasle
	if(found != keywords.size()){
		return false;
	}
	return true;
}

//searches based on the given phrase
bool phrase_search(const std::string& fileContent, const std::vector<std::string>& keywords){

	//makes the phrase
	std::string phrase = "";
	for(unsigned int i = 0; i < keywords.size(); i++){
		if(i == keywords.size()-1){
			phrase += keywords[i];
		}
		else{
			phrase += (keywords[i]+" ");
		}
	}

	//gets the body of the content
	std::string body;
	body = body_find(fileContent, body);

	//starts the searc from the begining of the body
	unsigned int start_pos = 0;

	//goes through the body until it finds the word or reaches the end
	while(start_pos <= body.length() && start_pos != std::string::npos){

		if(body.find(phrase, start_pos) == std::string::npos){
			return false;
		}

		//if the keyword is found without white space then add to found counter and continue
		else if(std::isspace(body[body.find(phrase, start_pos)-1]) == 0 ||
		std::isspace(body[body.find(phrase, start_pos)+phrase.length()]) == 0){

			//try the next section of the body
			start_pos = body.find(phrase, start_pos)+phrase.length();
			continue;
		}

		break;
	}
	return true;
}

//prints the matching documents
void print(const std::string& fileContent, const std::vector<std::string>& keywords, std::ofstream& ofile){

	//gets all the strings needed to print out
	std::string title;
	std::string URL;
	std::string description;
	std::string snippet = "";
	URL = URL_find(fileContent, URL);
	title = title_find(fileContent, title);
	description = description_find(fileContent, description);
	snippet = snippet_find(fileContent, keywords, snippet);

	//prints the strings
	ofile << "Title: " << title << std::endl;
	ofile << "URL: " << URL << std::endl;
	ofile << "Description: " << description << std::endl;
	ofile << "Snippet: " << snippet << std::endl;
}

int main(int argc, char* argv[]){

	//checks the amount of arguments given by the user
	if(argc < 4){
		std::cerr << "Not enough arguments, Goodbye." << std::endl;
		exit(1);
	}

	std::ofstream ofile(argv[2]);
	//checks if the out file opened correctly
  	if (!ofile.good()) { 
    	std::cerr << "Failed to open" << argv[2] << std::endl;
    	exit(1);
    }

    //determines if a phrase search is needed or not
    bool phrase = false;

    //gets a vector of keyword(s)
    std::vector<std::string> keywords;
    for(int i = 3; i < argc; i++){
    	std::string keyword = argv[i];
    	if(keyword.find("\"") != std::string::npos){
    		phrase = true;
    		keyword.erase(keyword.find("\""),1);
    	}
    	keywords.push_back(keyword);
    }

    //saves the start file as a string
    std::map<std::string, std::string> all_files;
    std::string start_file = argv[1];

    //gets all the links from the files provided into the map with the content as their value
    std::ifstream infile(argv[1]);
	if (infile.is_open() && infile.good()) {
		std::string fileContent((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

		//puts the ndex into the map first then calls web crawl on it's links
		std::string file1 = start_file.substr(start_file.rfind("/")+1);
		all_files[file1] = fileContent;
		std::list<std::string> links = extractLinksFromHTML(fileContent);
		std::list<std::string>::iterator link = links.begin();
		while(link != links.end()){
			web_crawl(fileContent, *link, all_files);
			link++;
		}
	}

	//map that will hold the page score as the double and the files content as the string
	std::map<double, std::string> output;

	//goes through the files and key words to get thier scores and puts them in the output map
	std::map<std::string, std::string>::iterator itr = all_files.begin();
	while(itr != all_files.end()){

		//gets the key density for the page by going through each keyword
		double keydens = 0;
		for(unsigned int i = 0; i < keywords.size(); i++){
			double tot_dens = total_keyword_density(all_files, keywords[i]);
			keydens += keyword_density(itr->second, keywords[i], tot_dens);
		}

		//gets the backdesnity and the page score
		double backdens = backlinks(itr->first, all_files);
		double pagescore = ((.5*keydens) + (.5*backdens));

		//adds to the output map if the phrase or key words are found in the body
		if(phrase){
			if(phrase_search(itr->second, keywords)){
				output[pagescore] = itr->second;
			}
		}
		else{
			if(regular_search(itr->second, keywords)){
				output[pagescore] = itr->second;
			}
		}
		itr++;
	}

	//prints if there are no matches
	if(output.size() == 0){
		ofile << "Your search - ";
		for(unsigned int i = 0; i < keywords.size(); i++){
			ofile << keywords[i]+" ";
		}
		ofile << "- did not match any documents." << std::endl;
		return 0;
	}

	//prints the matches if there are some
	ofile << "Matching documents: " << std::endl;
	std::map<double, std::string>::reverse_iterator out = output.rbegin();
	while(out != output.rend()){
		ofile << std::endl;
		print(out->second, keywords, ofile);
		out++;
	}

	return 0;
}