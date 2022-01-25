#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#define yaml_parse( p, e ) 		if( !yaml_parser_parse( &parser, &event ) ) { \
									printf("YAML Parser error %d\n", parser.error ); \
									exit(EXIT_FAILURE); }
/* Varaibles for statemachine in parsing */
static int map = 0;					/* flag to start checking for new map */
static int seq = 0;					/* flag to check if new sequence */
static int pkgnum = 0;				/* Current package number */
static int depnum = 0;				/* Current dependency number */
char** tmp_deplist = NULL;			/* Used for dynamic array reallocation for deplist */
struct pkg * tmp_pkglist = NULL;	/* Used for dynamic array reallocation for pkglist */
struct pkg {
	unsigned int major;				/* Package major version */
	unsigned int minor;				/* Package minor version */
	unsigned int subver;			/* Package subversion */
	unsigned int patch;				/* Package version patch */
	unsigned int depnum;			/* Number of dependencies */
	char* name;						/* Package name */
	char* filename;					/* Filename, though yaml will just provide extension */
	char* script;					/* Build script name or location */
	char** deplist;					/* List of dependencies required for package */

};

struct pkg * pkglist;

void parse_pkglist( char* filename);

int main(int argc, char** argv){

	pkglist = calloc( 1, sizeof(struct pkg));

	/* Open file */
	if( argc < 2 ){
		printf("No package list was provided\n");
	}
	
	parse_pkglist( argv[1] );



	printf("\nPackage Info:\n");
	/* print out pkg info */
	for( int i = 0; i < pkgnum; i++){
		printf("Name:\t\t\t%s\n", pkglist[i].name);
		printf("Filename:\t\t%s\n", pkglist[i].filename);
		printf("Version:\t\t%d.%d.%d-%d\n", pkglist[i].major, pkglist[i].minor, pkglist[i].subver, pkglist[i].patch);
		printf("Build script:\t\t%s\n", pkglist[i].script);
		printf("Num of Dependencies:\t%u\n", pkglist[i].depnum);
		for( int j = 0; j < pkglist[i].depnum; j++ ){
			printf("Dependency %d:\t\t%s\n", j, pkglist[i].deplist[j]);
		}
		printf("\n\n");
	}


/* Go here if there is an issue, makes it easier to clean up everything in one
 * spot */
cleanup:
	/* cleanup because otherwise this will get bad */
	for( int i = 0; i < pkgnum; i++){
		for( int j = 0; j < pkglist[i].depnum; j++){
			free( pkglist[i].deplist[j] );
		}
		free( pkglist[i].deplist );

		free( pkglist[i].script );
		free( pkglist[i].filename );
		free( pkglist[i].name );
	}

	free( pkglist );





	return 0;


}

void parse_pkglist( char* filename ){

	yaml_parser_t parser;
	yaml_event_t event;

	FILE *fpkglist = fopen(filename, "r");
	if( fpkglist == NULL ){
		printf("Could not open package list file: %s\n", filename);
	}

	/* Create parser */
	if( !yaml_parser_initialize(&parser) ){
		printf("Failed to initalize yaml parser\n");
	}
	
	/* Give file to yaml parser */
	yaml_parser_set_input_file( &parser, fpkglist );

	/* Start parsing out file */
	do {
		/* Check if parser failed */
		if( !yaml_parser_parse( &parser, &event ) ) {
			printf("YAML Parser error %d\n", parser.error );				
			exit(EXIT_FAILURE);
		}



		/* Actual parsing of file */
		switch( event.type ){
			case YAML_NO_EVENT:
				//printf("No event\n");
				break;
			case YAML_STREAM_START_EVENT:
				//printf("Stream Start\n");
				
				break;
			case YAML_STREAM_END_EVENT:
				//printf("Stream End\n");
				break;
			/* Block delimiters */
			case YAML_DOCUMENT_START_EVENT:
				//printf("\tStart document\n");
				break;
			case YAML_DOCUMENT_END_EVENT:
				//printf("\tEnd document\n");
				break;
			case YAML_SEQUENCE_START_EVENT:
				//printf("\t\tStart Sequence\n");
					
				seq++;
				/* Allocate resources for new sequence */
				
				break;
			case YAML_SEQUENCE_END_EVENT:
				//printf("\t\tEnd Sequence\n");
				
				/* if in dependency list, clear out count */
				if( map == 2 && seq == 2 ){
					depnum = 0;
				}

				seq--;
				break;
			case YAML_MAPPING_START_EVENT:
				//printf("\t\tMapping start\n");

				/* allocate resources for new mapping */
				map++;

				/* New package, allocate resources */
				if( (pkgnum > 0) && (map == 2 && seq == 1)){
					tmp_pkglist = reallocarray( pkglist, pkgnum+1, sizeof(struct pkg) );
					if( tmp_pkglist == NULL ){
						printf("Error changing size of package list\n");
						exit(1); 
					}
					else{
						pkglist = tmp_pkglist;
						tmp_pkglist = NULL;
					}

				}


				break;
			case YAML_MAPPING_END_EVENT:
				//printf("\t\tMapping end\n");
				
				/* if editing a package values, make sure to increment package count for next pacakge */
				if( map == 2 && seq == 1 ){
					pkgnum++;
					//printf("Finished package, increment count to %d\n", pkgnum);
				}

				map--;
				break;
			case YAML_ALIAS_EVENT:
				//printf("\t\t\tGot alias (anchor %s)\n", event.data.alias.anchor);
				break;
			case YAML_SCALAR_EVENT:
				//printf("\t\t\tMap: %u, sequence: %u.\tScalar: %s\n", map, seq, event.data.scalar.value);

				/* repository values */
				if( map == 1 && seq == 0){
					//printf("Repository data\n");
				}

				/* Package values */
				else if( map == 2 && seq == 1 ){
					/* Make sure to reset dependency number, as this would be a new package */

					if( strncmp( event.data.scalar.value, "name", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].name = calloc( event.data.scalar.length + 1, sizeof(char) );
						memcpy( pkglist[pkgnum].name, event.data.scalar.value, event.data.scalar.length );
					}	
					
					else if( strncmp( event.data.scalar.value, "major", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].major = atoi(event.data.scalar.value);
					}	
					else if( strncmp( event.data.scalar.value, "minor", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].minor = atoi(event.data.scalar.value);
					}	
					else if( strncmp( event.data.scalar.value, "subver", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].subver = atoi(event.data.scalar.value);
					}	
					else if( strncmp( event.data.scalar.value, "patch", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].patch = atoi(event.data.scalar.value);
					}	
					else if( strncmp( event.data.scalar.value, "script", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].script = calloc( event.data.scalar.length + 1, sizeof(char) );
						memcpy( pkglist[pkgnum].script, event.data.scalar.value, event.data.scalar.length );
					}	
					else if( strncmp( event.data.scalar.value, "filename", event.data.scalar.length) == 0){
						/* next scalar value will be the name */			
						yaml_parse( &parser, &event )
						pkglist[pkgnum].filename = calloc( event.data.scalar.length + 1, sizeof(char) );
						memcpy( pkglist[pkgnum].filename, event.data.scalar.value, event.data.scalar.length );
					}
					
				}
				else if( map == 2 && seq == 2 ){
					if( depnum == 0 ){
						/* Allocate for at least one pointer on the first call */
						pkglist[pkgnum].deplist = malloc( sizeof( char* ) );
						//printf("First allocation for dependency list\n");
					}
					else {
						//printf("Allocation %d for dependency list\n", depnum);
						/* Subsequent calls need to have reallocation */
						tmp_deplist = reallocarray( pkglist[pkgnum].deplist, depnum+1, sizeof(char *) );
						if( tmp_deplist == NULL ){
							printf("Error changing size of package dependency list\n");
							exit(1); 
						}
						else{
							pkglist[pkgnum].deplist = tmp_deplist;
							tmp_deplist = NULL;
						}
					}

					pkglist[pkgnum].deplist[depnum] = calloc(1, event.data.scalar.length + 1 );
					if( pkglist[pkgnum].deplist[depnum] == NULL){
						printf("Could not allocate memory for dependency string\n");
						exit(1);
					}
					memcpy( pkglist[pkgnum].deplist[depnum], event.data.scalar.value, event.data.scalar.length + 1);				

					//for( int j = 0; j < pkglist[pkgnum].depnum; j++ ){
					//	printf("Dependency %d:\t\t%s\n", j, pkglist[pkgnum].deplist[j]);
					//}
					depnum++;
					/* save dependency number in struct */
					pkglist[pkgnum].depnum = depnum;
				}

				break;
		}
		

		if( event.type != YAML_STREAM_END_EVENT ){
			yaml_event_delete( &event );
		}
	
	} while( event.type != YAML_STREAM_END_EVENT );

	yaml_event_delete( &event );

	/* Cleanup parser */
	yaml_parser_delete( &parser );
	fclose( fpkglist );






}
