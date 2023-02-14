const AWS = require('aws-sdk');
const ddb = new AWS.DynamoDB.DocumentClient({region: 'us-east-1'});

exports.handler = async (event, context, callback) => {
        var scan_result = await readMessage().then(data =>{
            return data.Items
    }).catch((err) =>{
        console.error(err);
    });
    scan_result = JSON.stringify(scan_result)
    scan_result = scan_result.split('"')
    scan_result = atob(scan_result)
    return{
        statusCode: 201,
        body: scan_result
    };
};

function readMessage(){
    const param = {
        TableName : 'fanTable',
    }
    return ddb.scan(param).promise();
}